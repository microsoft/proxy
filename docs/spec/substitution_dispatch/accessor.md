# Class template `substitution_dispatch::accessor`

```cpp
// (1)
template <class P, class D, class... Os>
struct accessor {
  accessor() = delete;
};

// (2)
template <class P, class D, class... Os>
    requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<P, D, Os>> && ...))
struct accessor<P, D, Os...> : accessor<P, D, Os>... {
  using accessor<P, D, Os>::operator return-type-of<Os>...;
};

// (3)
template <class P, class D, facade F>
struct accessor<P, D, proxy<F>() cv ref noex> {
  operator proxy<F>() cv ref noex;
};
```

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<P, D, Os>...` are default-constructible, inherits all `accessor<P, D, Os>...` types and `using` their `operator return-type-of<Os>`. `return-type-of<O>` denotes the *return type* of the overload type `O`.

`(3)` When `sizeof...(Os)` is `1` and the only type `O` in `Os` is `proxy<F>() cv ref noex`, provides an implicit  `operator proxy<F>()` with the same *cv ref noex* specifiers. `accessor::operator proxy<F>()` is equivalent to `return proxy_invoke<proxy<F>() cv ref noex>(static_cast<P cv <ref ? ref : &>>(*this))` when `static_cast<const P&>(*this).has_value()` is `true`, or `return nullptr` otherwise.
