# Class template `conversion_dispatch::accessor`

```cpp
// (1)
template <class F, class C, class... Os>
struct accessor {
  accessor() = delete;
};

// (2)
template <class F, class C, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_trivial_v<accessor<F, C, Os>> && ...))
struct accessor<F, C, Os...> : accessor<F, C, Os>... {
  using accessor<F, C, Os>::operator T...;
};

// (3)
template <class F, class C>
struct accessor<F, C, T() cv ref noex> {
  explicit(Expl) operator T() cv ref noex;
};
```

Let `SELF` be `std::forward<accessor cv ref>(*this)`.

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, C, Os>...` are trivial types, inherits all `accessor<F, C, Os>...` types and `using` their `operator T`.

`(3)` When `sizeof...(Os)` is `1` and the only type `O` in `Os` is `T() cv ref noex`, provides an explicit (when `Expl` is `true`) or implicit (when `Expl` is `false`) `operator T()` with the same *cv ref noex* specifiers. `accessor::operator T()` is equivalent to `return proxy_invoke<C, T() cv ref noex>(access_proxy<F>(SELF))`.
