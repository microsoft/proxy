# `operator_dispatch::operator()`

For different `Sign` and `Rhs`, `operator_dispatch<Sign, Rhs>::operator()` are defined differently. `sop` denotes the sign of operator of each specialization.

## Left-Hand-Side Operand Specializations

When `Rhs` is `false`, `operator()` of the specializations are defined as follows:

### Unary & Binary SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"&"`,

```cpp
// (1)
template <class T>
/* see below */ operator()(T&& self)
    noexcept(noexcept(sop std::forward<T>(self)))
    requires(requires { sop std::forward<T>(self); });

// (2)
template <class T, class Arg>
/* see below */ operator()(T&& self, Arg&& arg)
    noexcept(noexcept(std::forward<T>(self) sop std::forward<Arg>(arg)))
    requires(requires { std::forward<T>(self) sop std::forward<Arg>(arg); });
```

`(1)` Returns `sop std::forward<T>(self)`.

`(2)` Returns `std::forward<T>(self) sop std::forward<Arg>(arg)`.

### Binary SOPs

When `Sign` is one of `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"|"`, `"^"`, `"<<"`, `">>"`, `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`, `","`, `"->*"`,

```cpp
// (3)
template <class T, class Arg>
/* see below */ operator()(T&& self, Arg&& arg)
    noexcept(noexcept(std::forward<T>(self) sop std::forward<Arg>(arg)))
    requires(requires { std::forward<T>(self) sop std::forward<Arg>(arg); });
```

`(3)` Returns `std::forward<T>(self) sop std::forward<Arg>(arg)`.

### `"++"` and `"--"`

When `Sign` is either `"++"`, `"--"`,

```cpp
// (4)
template <class T>
/* see below */ operator()(T&& self)
    noexcept(noexcept(sop std::forward<T>(self)))
    requires(requires { sop std::forward<T>(self); });

// (5)
template <class T>
/* see below */ operator()(T&& self, int)
    noexcept(noexcept(std::forward<T>(self) sop))
    requires(requires { std::forward<T>(self) sop; });
```

`(4)` Returns `sop std::forward<T>(self)`.

`(5)` Returns `std::forward<T>(self) sop`.

### `"!"` and `"~"`

When `Sign` is either `"!"`, `"~"`,

```cpp
// (6)
template <class T>
/* see below */ operator()(T&& self)
    noexcept(noexcept(sop std::forward<T>(self)))
    requires(requires { sop std::forward<T>(self); });
```

`(6)` Returns `sop std::forward<T>(self)`.

### `"()"`

When `Sign` is `"()"`,

```cpp
// (7)
template <class T, class... Args>
/* see below */ operator()(T&& self, Args&&... args)
    noexcept(noexcept(std::forward<T>(self)(std::forward<Args>(args)...)))
    requires(requires { std::forward<T>(self)(std::forward<Args>(args)...); });
```

`(7)` Returns `std::forward<T>(self)(std::forward<Args>(args)...)`.


### `"[]"`

When `Sign` is `"[]"`,

```cpp
// (8) (until C++23)
template <class T, class... Args>
/* see below */ operator()(T&& self, Args&&... args)
    noexcept(noexcept(std::forward<T>(self)[std::forward<Args>(args)...]))
    requires(requires { std::forward<T>(self)[std::forward<Args>(args)...]; });

// (9) (since C++23)
template <class T, class Arg>
/* see below */ operator()(T&& self, Arg&& arg)
    noexcept(noexcept(std::forward<T>(self)[std::forward<Arg>(arg)]))
    requires(requires { std::forward<T>(self)[std::forward<Arg>(arg)]; });
```

`(8)` Returns `std::forward<T>(self)[std::forward<Args>(args)...]`. Requires the [multidimensional subscript feature](https://en.cppreference.com/w/cpp/language/operators#Array_subscript_operator) in C++23.

`(9)` Returns `std::forward<T>(self)[std::forward<Arg>(arg)]`.

## Right-Hand-Side Operand Specializations

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`, `","`, `"->*"`, and `Rhs` is `true`,

```cpp
// (10)
template <class T, class Arg>
/* see below */ operator()(T&& self, Arg&& arg)
    noexcept(noexcept(std::forward<Arg>(arg) sop std::forward<T>(self)))
    requires(requires { std::forward<Arg>(arg) sop std::forward<T>(self); });
```

`(10)` Returns `std::forward<Arg>(arg) sop std::forward<T>(self)`.
