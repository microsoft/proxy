# Class template `operator_dispatch::accessor`

```cpp
// (1)
template <class F, class C, class... Os>
struct accessor {
  accessor() = delete;
};
```

`(1)` The default implementation of `accessor` is not constructible.

For different `Sign` and `Rhs`, `operator_dispatch<Sign, Rhs>::accessor` has different specializations. `sop` denotes the sign of operator of each specialization.

## Left-Hand-Side Operand Specializations

```cpp
// (2)
template <class F, class C, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_trivial_v<accessor<F, C, Os>> && ...))
struct accessor<F, C, Os...> : accessor<F, C, Os>... {
  using accessor<F, C, Os>::operator sop...;
};
```

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, C, Os>...` are trivial types, inherits all `accessor<F, C, Os>...` types and `using` their `operator sop`.

When `Rhs` is `false`, the other specializations are defined as follows, where `sizeof...(Os)` is `1` and the only type `O` qualified with `cv ref noex` (let `SELF` be `std::forward<accessor cv ref>(*this)`):

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"++"`, `"--"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`, `"()"`, `"[]"`,

```cpp
// (3)
template <class F, class C, class R, class... Args>
struct accessor<F, C, R(Args...) cv ref noex> {
  R operator sop (Args... args) cv ref noex;
}
```

`(3)` Provides an `operator sop(Args...)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Args...)` is equivalent to `return proxy_invoke<C, R(Args...) cv ref noex>(access_proxy<F>(SELF), std::forward<Args>(args)...)`.

### `!` and `~`

When `Sign` is either `!` and `~`,

```cpp
// (4)
template <class F, class C, class R>
struct accessor<F, C, R() cv ref noex> {
  R operator sop () cv ref noex;
}
```

`(4)` Provides an `operator sop()` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop()` is equivalent to `return proxy_invoke<C, R() cv ref noex>(access_proxy<F>(SELF))`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (5)
template <class F, class C, class R, class Arg>
struct accessor<F, C, R(Arg) cv ref noex> {
  /* see below */ operator sop (Arg arg) cv ref noex;
}
```

`(4)` Provides an `operator sop(Arg)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Arg)` calls `proxy_invoke<C, R(Arg) cv ref noex>(access_proxy<F>(SELF), std::forward<Arg>(arg))` and returns `access_proxy<F>(SELF)` when `C::is_direct` is `true`, or otherwise, returns `*access_proxy<F>(SELF)` when `C::is_direct` is `false`.

## Right-Hand-Side Operand Specializations

```cpp
// (6)
template <class F, class C, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_trivial_v<accessor<F, C, Os>> && ...))
struct accessor<F, C, Os...> : accessor<F, C, Os>... {};
```

`(6)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, C, Os>...` are trivial types, inherits all `accessor<F, C, Os>...` types.

When `Rhs` is `true`, the other specializations are defined as follows, where `sizeof...(Os)` is `1` and the only type `O` qualified with `cv ref noex` (let `SELF` be `std::forward<accessor cv ref>(self)`):

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`,

```cpp
// (7)
template <class F, class C, class R, class Arg>
struct accessor<F, C, R(Arg) cv ref noex> {
  friend R operator sop (Arg arg, accessor cv ref self) noex;
}
```

`(7)` Provides a `friend operator sop(Arg arg, accessor cv ref)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg arg, accessor cv ref)` is equivalent to `return proxy_invoke<C, R(Arg) cv ref noex>(access_proxy<F>(SELF), std::forward<Arg>(arg))`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (8)
template <class F, class C, class R, class Arg>
struct accessor<F, C, R(Arg) cv ref noex> {
  friend /* see below */ operator sop (Arg arg, accessor cv ref self) noex;
}
```

`(8)` Provides a `friend operator sop(Arg arg, accessor cv ref)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg arg, accessor cv ref)` calls `proxy_invoke<C, R(Arg) cv ref noex>(access_proxy<F>(SELF), std::forward<Arg>(arg))` and returns `access_proxy<F>(SELF)` when `C::is_direct` is `true`, or otherwise, returns `*access_proxy<F>(SELF)` when `C::is_direct` is `false`.
