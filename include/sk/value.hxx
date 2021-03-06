/*
 * Copyright (c) 2019, 2020, 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SK_VALUE_HXX_INCLUDED
#define SK_VALUE_HXX_INCLUDED

#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

/*
 * value - type-erased polymorphic scalars.
 */

namespace sk {

    struct value;

    // clang-format off
    template<typename T>
    concept value_containable = 
        not std::same_as<typename std::remove_cvref<T>::type, value>
        and requires (T &o) {
        { std::hash<T>{}(o) } -> std::same_as<std::size_t>;
    };

    template<typename T>
    concept value_printable = requires (T &o, std::ostream &strm) {
        { strm << o } -> std::same_as<std::ostream &>;
    };

    template<typename T>
    concept value_lt_comparable = requires(T &o) {
        { o < o } -> std::same_as<bool>;
    };

    // clang-format on

    template <value_containable T>
    auto value_containable_to_string(T const &v) -> std::string
        requires value_printable<T> {
        std::ostringstream strm;
        strm << v;
        return strm.str();
    }

    template <value_containable T>
    auto value_containable_to_string(T const &v) -> std::string
        requires !value_printable<T> {
        return "<value>";
    }

    template <value_containable T>
    auto value_lt_compare(T const &a, T const &b)
        -> bool requires value_lt_comparable<T> {
        return a < b;
    }

    template <value_containable T>
    auto value_lt_compare(T const &a, T const &b)
        -> bool requires !value_lt_comparable<T> {
        return false;
    }

    struct value_base {
        virtual ~value_base() = default;
        virtual auto copy() const -> std::unique_ptr<value_base> = 0;
        virtual auto hash() const -> std::size_t = 0;
        virtual auto str() const -> std::string = 0;
        virtual auto eq(value_base const *other) const -> bool = 0;
        virtual auto lt(value_base const *other) const -> bool = 0;
    };

    template <typename T> struct value_instance final : value_base {
        T object;

        // Construct a new value.
        template <typename... Args>
        explicit value_instance(Args &&...args)
            : object(std::forward<Args>(args)...) {}

        auto copy() const -> std::unique_ptr<value_base> final {
            return std::make_unique<value_instance>(object);
        }

        auto hash() const -> std::size_t final {
            return std::hash<T>{}(object);
        }

        auto str() const -> std::string final {
            return value_containable_to_string(object);
        }

        auto eq(value_base const *other) const -> bool final {
            auto const *p = dynamic_cast<value_instance<T> const *>(other);
            if (!p)
                return false;
            return object == p->object;
        }

        auto lt(value_base const *other) const -> bool final {
            auto const *p = dynamic_cast<value_instance<T> const *>(other);
            if (!p)
                return false;
            return value_lt_compare(object, p->object);
        }
    };

    /*
     * The value.
     */
    struct value {
        // Create an empty value.
        value()
            : object(std::make_unique<value_instance<nullptr_t>>(nullptr)) {}

        // Create a value from a value_containable.
        template <typename T>
            explicit value(T &&v) requires value_containable<
                typename std::remove_cvref<T>::type>
            : object(std::make_unique<
                     value_instance<typename std::remove_cvref<T>::type>>(
                  std::forward<T>(v))) {}

        // A value created from a C string should be stored
        // as an std::basic_string for consistency.
        explicit value(char const *s)
            : object(std::make_unique<value_instance<std::string>>(s)) {}
        explicit value(wchar_t const *s)
            : object(std::make_unique<value_instance<std::wstring>>(s)) {}
        explicit value(char8_t const *s)
            : object(std::make_unique<value_instance<std::u8string>>(s)) {}
        explicit value(char16_t const *s)
            : object(std::make_unique<value_instance<std::u16string>>(s)) {}
        explicit value(char32_t const *s)
            : object(std::make_unique<value_instance<std::u32string>>(s)) {}

        // Copy a value.
        value(value const &other)
            : object(other.object ? other.object->copy() : nullptr) {}

        // Move a value
        value(value &&other) noexcept : object(std::move(other.object)) {}

        // Assign a value from a value_containable.
        template <typename T>
        auto operator=(T &&v) -> value
            &requires value_containable<typename std::remove_cvref<T>::type> {

            object = std::make_unique<
                value_instance<typename std::remove_cvref<T>::type>>(
                std::forward<T>(v));
            return *this;
        }

        auto operator=(char const *s) -> value & {
            object = std::make_unique<value_instance<std::string>>(s);
        }

        auto operator=(wchar_t const *s) -> value & {
            object = std::make_unique<value_instance<std::wstring>>(s);
        }

        auto operator=(char8_t const *s) -> value & {
            object = std::make_unique<value_instance<std::u8string>>(s);
        }

        auto operator=(char16_t const *s) -> value & {
            object = std::make_unique<value_instance<std::u16string>>(s);
        }

        auto operator=(char32_t const *s) -> value & {
            object = std::make_unique<value_instance<std::u32string>>(s);
        }

        auto operator=(value const &other) {
            object = other.object ? other.object->copy() : nullptr;
        }

        auto operator=(value &&other) noexcept {
            object = std::move(other.object);
        }

        // The stored value.
        std::unique_ptr<value_base> object;

        auto empty() const -> bool {
            static value_instance<nullptr_t> null_value;
            return object->eq(&null_value);
        }

        auto str() const -> std::string {
            return object->str();
        }
    };

    template <value_containable To>
    auto value_cast(value const *from) -> To const * {
        if (from->empty())
            return nullptr;

        auto const *o =
            dynamic_cast<value_instance<To> const *>(from->object.get());

        return o ? &o->object : nullptr;
    }

    template <value_containable To>
    auto value_cast(value const &from) -> To const & {
        auto const &o = dynamic_cast<value_instance<To> const &>(*from.object);
        return o.object;
    }

    inline auto operator==(value const &a, value const &b) -> bool {
        if (a.empty() or b.empty()) {
            if (a.empty() and b.empty())
                return true;
            return false;
        }

        return a.object->eq(b.object.get());
    }

    template <value_containable T>
    inline auto operator==(value const &a, T const &b) -> bool {
        auto const *p = value_cast<T>(&a);
        if (!p)
            return false;
        return *p == b;
    }

    template <value_containable T>
    inline auto operator==(T const &a, value const &b) -> bool {
        return b == a;
    }

    inline auto operator==(value const &a, nullptr_t) -> bool {
        return a.empty();
    }

    inline auto operator==(nullptr_t, value const &b) -> bool {
        return b.empty();
    }

    inline auto operator==(value const &a, char const *b) -> bool {
        return a == std::string(b);
    }

    inline auto operator==(value const &a, wchar_t const *b) -> bool {
        return a == std::wstring(b);
    }

    inline auto operator==(value const &a, char8_t const *b) -> bool {
        return a == std::u8string(b);
    }

    inline auto operator==(value const &a, char16_t const *b) -> bool {
        return a == std::u16string(b);
    }

    inline auto operator==(value const &a, char32_t const *b) -> bool {
        return a == std::u32string(b);
    }

    inline auto operator==(char const *a, value const &b) -> bool {
        return b == a;
    }

    inline auto operator<(value const &a, value const &b) -> bool {
        if (a.empty()) {
            if (b.empty())
                return false;
            return true;
        }

        if (b.empty())
            return false;

        auto const &ai = typeid(*a.object), &bi = typeid(*b.object);
        if (ai != bi)
            return ai.before(bi);
        return a.object->lt(b.object.get());
    }

    /*
     * ostream output support.
     */
    inline auto operator<<(std::ostream &strm, sk::value const &v) -> std::ostream & {
        strm << v.str();
        return strm;
    }

} // namespace sk

/*
 * std::hash<> support.
 */
template <> struct std::hash<sk::value> {
    std::size_t operator()(sk::value const &v) const {
        return v.object->hash();
    }
};

#endif // SK_VALUE_HXX_INCLUDED
