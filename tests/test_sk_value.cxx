/*
 * Copyright(c) 2019, 2020, 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license(the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third - parties to whom the Software is furnished to
 * do so, all subject to the following :
 *
 * The copyright notices in the Softwareand this entire statement, including
 * the above license grant, this restrictionand the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine - executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

#include "sk/value.hxx"

TEST_CASE("value operator<") {
    sk::value v1{1}, v2{2}, vstr{std::string("foo")}, vempty1, vempty2;

    REQUIRE(v1 < v2);
    REQUIRE(((v1 < vstr) || (vstr < v1)));
    REQUIRE(!(v1 < vstr && vstr < v1));
    REQUIRE(!(v2 < v1));
    REQUIRE(vempty1 < v1);
    REQUIRE(vempty1 < v2);
    REQUIRE(!(vempty1 < vempty2));
}

TEST_CASE("value operator==") {
    sk::value v42{42}, vstr{"foo"};

    REQUIRE(v42 == 42);
    REQUIRE(v42 == v42);
    REQUIRE(v42 != vstr);
    REQUIRE(vstr != v42);
    REQUIRE(vstr == "foo");
    REQUIRE("foo" == vstr);
}

TEST_CASE("value operator<<") {
    sk::value v42{42}, vstr{"foo"};

    std::ostringstream strm;
    strm << v42;
    REQUIRE(strm.str() == "42");

    strm.str("");
    strm << vstr;
    REQUIRE(strm.str() == "foo");
}

TEST_CASE("value::str()") {
    sk::value v42{42};
    REQUIRE(v42.str() == "42");

    sk::value vstr{"foo"};
    REQUIRE(vstr.str() == "foo");
}

TEST_CASE("char const * literal value") {
    sk::value vstr{"foo"};
    std::string const *s = sk::value_cast<std::string>(&vstr);
    REQUIRE(*s == "foo");
}

TEST_CASE("int value") {
    // Construct
    sk::value v{42};
    REQUIRE(v == 42);
    REQUIRE(42 == v);
    REQUIRE(v != 42.5);
    REQUIRE(v.str() == "42");
    REQUIRE(!v.empty());

    // Hash
    REQUIRE(std::hash<sk::value>{}(v) == std::hash<int>{}(42));

    // Copy
    sk::value v2{v};
    REQUIRE(!v2.empty());
    REQUIRE(v.str() == "42");
    REQUIRE(v2.str() == "42");

    // Cast to pointer.
    int const *iptr = sk::value_cast<int>(&v);
    REQUIRE(*iptr == 42);

    double const *dptr = sk::value_cast<double>(&v);
    REQUIRE(dptr == nullptr);

    // Cast to reference.
    int const &iref = sk::value_cast<int>(v);
    REQUIRE(iref == 42);
}

TEST_CASE("empty value") {
    sk::value v;
    REQUIRE(v.empty());
    REQUIRE(v == nullptr);
    REQUIRE(v != 42);

    sk::value v2{v};
    REQUIRE(v2.empty());

    REQUIRE(v == v2);
    REQUIRE(!(v < v2));
    REQUIRE(!(v2 < v));
}

TEST_CASE("value reference construction") {
    std::string s;
    std::string const &sref = s;

    sk::value v{s};
    sk::value vref{sref};
    v = s;
    v = sref;
}
