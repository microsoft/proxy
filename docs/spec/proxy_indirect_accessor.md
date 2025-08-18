# Class template `proxy_indirect_accessor`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
template <facade F>
class proxy_indirect_accessor;
```

Class template `proxy_indirect_accessor` provides indirection accessibility for `proxy`. As per `facade<F>`, `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Cs`, and `typename F::reflection_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Rs`.

- For each type `C` in `Cs`, if `C::is_direct` is `false` and `typename C::dispatch_type` meets the [*ProAccessible* requirements](ProAccessible.md) of `proxy_indirect_accessor<F>, typename C::dispatch_type, substituted-overload-types...`, `typename C::dispatch_type::template accessor<proxy<F>, typename C::dispatch_type, substituted-overload-types...>` is inherited by `proxy_indirect_accessor<F>`. Let `Os...` be the element types of `typename C::overload_types`, `substituted-overload-types...` is [`substituted-overload<Os, F>...`](ProOverload.md).
- For each type `R` in `Rs`, if `R::is_direct` is `false` and `typename R::reflector_type` meets the [*ProAccessible* requirements](ProAccessible.md) of `proxy_indirect_accessor<F>, typename R::reflector_type`, `typename R::reflector_type::template accessor<proxy_indirect_accessor<F>, typename R::reflector_type` is inherited by `proxy_indirect_accessor<F>`.

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## See also

- [function template `proxy_invoke`](proxy_invoke.md)
- [function template `proxy_reflect`](proxy_reflect.md)
