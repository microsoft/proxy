# `implicit_conversion_dispatch::operator()`

```cpp
template <class T>
T&& operator()(T&& value) noexcept;
```

Returns `std::forward<T>(value)`.
