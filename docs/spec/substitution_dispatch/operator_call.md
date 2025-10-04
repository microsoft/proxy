# `substitution_dispatch::operator()`

```cpp
template <class T>
T&& operator()(T&& value) const noexcept; // exposition-only
```

Returns `std::forward<T>(value)`. When `T` is not cv- or ref-qualified and [`is_bitwise_trivially_relocatable_v<T>`](../is_bitwise_trivially_relocatable.md) is `true`, conversion from the return value to any `proxy` type shall perform bitwise trivial relocation and does not require that `T` is move-constructible.
