# `proxy::swap`

```cpp
void swap(proxy& rhs)
    noexcept(F::constraints.relocatability >= constraint_level::nothrow ||
        F::constraints.copyability == constraint_level::trivial)
    requires(F::constraints.relocatability >= constraint_level::nontrivial ||
        F::constraints.copyability == constraint_level::trivial);
```

Exchanges the contained values of `*this` and `rhs`.
