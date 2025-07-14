# `proxy::swap`

```cpp
void swap(proxy& rhs)
    noexcept(F::relocatability >= constraint_level::nothrow ||
        F::copyability == constraint_level::trivial)
    requires(F::relocatability >= constraint_level::nontrivial ||
        F::copyability == constraint_level::trivial);
```

Exchanges the contained values of `*this` and `rhs`.
