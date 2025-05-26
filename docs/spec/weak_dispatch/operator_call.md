# `weak_dispatch<D>::operator()`

```cpp
// (1)
using D::operator();

// (2)
template <class... Args>
[[noreturn]] /* see below */ operator()(std::nullptr_t, Args&&...) const;
```

- `(1)` Forwards an invocation to `D` if applicable.
- `(2)` Throws [`not_implemented`](../not_implemented.md). The return type is convertible to any type to match a arbitrary convention.
