# Class template `operator_dispatch::accessor`

```cpp
// (1)
template <class F, bool IsDirect, class D, class... Os>
struct accessor {
  accessor() = delete;
};
```

`(1)` The default implementation of `accessor` is not constructible.

For different `Sign` and `Rhs`, `operator_dispatch<Sign, Rhs>::accessor` has different specializations. `sop` denotes the sign of operator of each specialization.

## Left-Hand-Side Operand Specializations

```cpp
// (2)
template <class F, bool IsDirect, class D, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<F, IsDirect, D, Os>> && ...))
struct accessor<F, IsDirect, D, Os...> : accessor<F, IsDirect, D, Os>... {
  using accessor<F, IsDirect, D, Os>::operator sop...;
};
```

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, IsDirect, D, Os>...` are default-constructible types, inherits all `accessor<F, IsDirect, D, Os>...` types and `using` their `operator sop`.

When `Rhs` is `false`, the other specializations are defined as follows, where `sizeof...(Os)` is `1` and the only type `O` qualified with `cv ref noex` (let `ACCESS_PROXY_EXPR` be `access_proxy<F>(std::forward<accessor cv ref>(*this))`):

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"++"`, `"--"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`, `"()"`, `"[]"`,

```cpp
// (3)
template <class F, bool IsDirect, class D, class R, class... Args>
struct accessor<F, IsDirect, D, R(Args...) cv ref noex> {
  R operator sop (Args... args) cv ref noex;
}
```

`(3)` Provides an `operator sop(Args...)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Args...)` is equivalent to `return proxy_invoke<IsDirect, D, R(Args...) cv ref noex>(ACCESS_PROXY_EXPR, std::forward<Args>(args)...)`.

### `!` and `~`

When `Sign` is either `!` and `~`,

```cpp
// (4)
template <class F, bool IsDirect, class D, class R>
struct accessor<F, IsDirect, D, R() cv ref noex> {
  R operator sop () cv ref noex;
}
```

`(4)` Provides an `operator sop()` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop()` is equivalent to `return proxy_invoke<IsDirect, D, R() cv ref noex>(ACCESS_PROXY_EXPR)`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (5)
template <class F, bool IsDirect, class D, class R, class Arg>
struct accessor<F, IsDirect, D, R(Arg) cv ref noex> {
  /* see below */ operator sop (Arg arg) cv ref noex;
}
```

`(4)` Provides an `operator sop(Arg)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Arg)` calls `proxy_invoke<IsDirect, D, R(Arg) cv ref noex>(ACCESS_PROXY_EXPR, std::forward<Arg>(arg))` and returns `ACCESS_PROXY_EXPR` when `IsDirect` is `true`, or otherwise, returns `*ACCESS_PROXY_EXPR` when `IsDirect` is `false`.

## Right-Hand-Side Operand Specializations

```cpp
// (6)
template <class F, bool IsDirect, class D, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<F, IsDirect, D, Os>> && ...))
struct accessor<F, IsDirect, D, Os...> : accessor<F, IsDirect, D, Os>... {};
```

`(6)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, IsDirect, D, Os>...` are default-constructible types, inherits all `accessor<F, IsDirect, D, Os>...` types.

When `Rhs` is `true`, the other specializations are defined as follows, where `sizeof...(Os)` is `1` and the only type `O` qualified with `cv ref noex` (let `accessor_arg` be `std::conditional_t<IsDirect, proxy<F>, proxy_indirect_accessor<F>>`, `ACCESS_PROXY_EXPR` be `access_proxy<F>(std::forward<accessor_arg cv ref>(self))`):

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`,

```cpp
// (7)
template <class F, bool IsDirect, class D, class R, class Arg>
struct accessor<F, IsDirect, D, R(Arg) cv ref noex> {
  friend R operator sop (Arg arg, accessor_arg cv ref self) noex;
}
```

`(7)` Provides a `friend operator sop(Arg arg, accessor_arg cv ref self)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg arg, accessor_arg cv ref self)` is equivalent to `return proxy_invoke<IsDirect, D, R(Arg) cv ref noex>(ACCESS_PROXY_EXPR, std::forward<Arg>(arg))`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (8)
template <class F, bool IsDirect, class D, class R, class Arg>
struct accessor<F, IsDirect, D, R(Arg) cv ref noex> {
  friend /* see below */ operator sop (Arg arg, accessor_arg cv ref self) noex;
}
```

`(8)` Provides a `friend operator sop(Arg arg, accessor_arg cv ref self)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg arg, accessor_arg cv ref self)` calls `proxy_invoke<IsDirect, D, R(Arg) cv ref noex>(ACCESS_PROXY_EXPR, std::forward<Arg>(arg))` and returns `ACCESS_PROXY_EXPR` when `IsDirect` is `true`, or otherwise, returns `*ACCESS_PROXY_EXPR` when `IsDirect` is `false`.
