# Concept `facade`

```cpp
template <class F>
concept facade = /* see-below */;
```

The concept `facade<F>` specifies that a type `F` models a facade of [`proxy`](proxy.md). If `F` depends on an incomplete type, and its evaluation could yield a different result if that type were hypothetically completed, the behavior is undefined. `facade<F>` is `true` when `F` meets the [*ProBasicFacade* requirements](ProBasicFacade.md); otherwise, it is `false`.

Note that concept `facade` does not impose strong constraints on the dependent convention and reflection types. It is recommended to use [`facade_builder`](basic_facade_builder.md) to define a facade type that models concept `facade`.

## See Also

- [class template `basic_facade_builder`](basic_facade_builder.md)
- [concept `proxiable`](proxiable.md)
- [function template `make_proxy`](make_proxy.md)
