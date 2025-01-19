# `explicit_conversion_dispatch::operator()`

```cpp
template <class T>
/* see below */ operator()(T&& value) const noexcept;
```

Returns a value that is implicitly convertible to any type `U` with expression `U{std::forward<T>(value)}` when `T` is explicitly convertible to type `U`.
