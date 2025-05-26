# `implicit_conversion_dispatch::operator()`

```cpp
template <class T>
T&& operator()(T&& value) const noexcept;
```

Returns `std::forward<T>(value)`.
