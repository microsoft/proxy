# Class template `implicit_conversion_dispatch::accessor`

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
template <class P, class D, class T>
struct accessor<P, D, T() cv ref noex> {
  operator T() cv ref noex;
};
```

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Os)` is greater than `1`, and `accessor<P, D, Os>...` are default-constructible, inherits all `accessor<P, D, Os>...` types and `using` their `operator return-type-of<Os>`. `return-type-of<O>` denotes the *return type* of the overload type `O`.

`(3)` When `sizeof...(Os)` is `1` and the only type `O` in `Os` is `T() cv ref noex`, provides an implicit  `operator T()` with the same *cv ref noex* specifiers. `accessor::operator T()` is equivalent to `return proxy_invoke<T() cv ref noex>(static_cast<P cv <ref ? ref : &>>(*this))`.
