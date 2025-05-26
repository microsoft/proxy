# Class template `explicit_conversion_dispatch::accessor`

```cpp
// (1)
template <class F, bool IsDirect, class D, class... Os>
struct accessor {
  accessor() = delete;
};

// (2)
template <class F, bool IsDirect, class D, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<F, IsDirect, D, Os>> && ...))
struct accessor<F, IsDirect, D, Os...> : accessor<F, IsDirect, D, Os>... {
  using accessor<F, IsDirect, D, Os>::operator return-type-of<Os>...;
};

// (3)
template <class F, bool IsDirect, class D>
struct accessor<F, IsDirect, D, T() cv ref noex> {
  explicit operator T() cv ref noex;
};
```

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<F, IsDirect, D, Os>...` are default-constructible, inherits all `accessor<F, IsDirect, D, Os>...` types and `using` their `operator return-type-of<Os>`. `return-type-of<O>` denotes the *return type* of the overload type `O`.

`(3)` When `sizeof...(Os)` is `1` and the only type `O` in `Os` is `T() cv ref noex`, provides an explicit  `operator T()` with the same *cv ref noex* specifiers. `accessor::operator T()` is equivalent to `return proxy_invoke<IsDirect, D, T() cv ref noex>(access_proxy<F>(std::forward<accessor cv ref>(*this)))`.
