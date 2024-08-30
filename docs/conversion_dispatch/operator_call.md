# `conversion_dispatch::operator()`

```cpp
template <class U>
T operator()(U&& value)
    noexcept(std::conditional_t<Expl, std::is_nothrow_constructible<T, U>,
        std::is_nothrow_convertible<U, T>>::value)
    requires(std::conditional_t<Expl, std::is_constructible<T, U>,
        std::is_convertible<U, T>>::value);
```

Converts `value` to type `T`. When `Expl` is `true`, `std::is_constructible<T, U>` is required to be `true`, or otherwise when `Expl` is `false`, `std::is_convertible<U, T>` is required to be `true`.

Returns `static_cast<T>(std::forward<U>(value))`.
