# `sk::value`

`sk::value` is a type-erasing container for storing scalar values.
It is similar to `std::any`, except that `sk::value`:

* Is ordered.
* Is equality comparable.
* Can be hashed.
* Can be printed to a stream.
* Can be converted to a string.

In most cases, this is not the container you want, and you should use `std::any`
or `std::variant` instead.  We use `sk::value` internally to represent values
retrieved from a database or file, when those values need to be convertible to
strings or converted back to the source types or related types to e.g. pass them
to a script interpreter engine.

`sk::value` cannot store objects which do not match its own capabilities; for
example, it cannot store non-copyable objects or non-comparable objects.
The exception is that non-printable objects can be stored; trying to convert
them to a string will return "&lt;value&gt;".

## Sample usage

```c++
int main() {
	// Create a value holding the number 42.
	sk::value v{42};

	assert(!v.empty());

	// Prints "42".
	std::cout << v << '\n';

	// Also prints "42".
	std::cout << v.str() << '\n';

	// Retrieve the int value of the stored object.
	int const *iptr = sk::value_cast<int>(&v);
	assert(*iptr == 42);

	// Error: cannot change the value of the stored object.
	*iptr = 666;

	// uiptr is nullptr, because int and unsigned int are different types.
	// There is no implicit conversion between integer types.
	unsigned int *uiptr = sk::value_cast<unsigned int>(&v);

	// May work or may throw std::bad_cast, depending on whether int and
	// int32_t are the same type on this implementation.
	std::int32_t i32 = sk::value_cast<int>(v);

	// To avoid that problem, store integers as specific types.
	sk::value v_i32{std::int32_t(42)};
	std::int32_t *i32ptr = sk::value_cast<std::int32_t>(&v_i32);
	assert(*i32ptr == 42); // ok

	// Creating a value from a C string stores an std::basic_string instead.
	sk::value v_str{"This is a string"};
	std::string const &sref = sk::value_cast<std::string>(v);
	assert(sref == "This is a string"); // ok 
	// This also works with wchar_t, char8_t, char16_t and char32_t.  However
	// due to lack of proper Unicode support in C++20, only std::string can be
	// converted back to a string with .str().  For other string types, you
	// will have to retrieve the string object with sk::value_cast<>.
}
```
