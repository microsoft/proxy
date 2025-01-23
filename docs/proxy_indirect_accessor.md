# Class template `proxy_indirect_accessor`

```cpp
template <class F>
class proxy_indirect_accessor;  // since 3.2
```

Class template `proxy_indirect_accessor` provides indirection accessibility for `proxy`. To instantiate `proxy_indirect_accessor<F>`, `F` shall model [concept `facade`](facade.md). As per `facade<F>`, `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Cs`, and `typename F::reflection_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Rs`. For each type `T` in `Cs` or `Rs`, if `T` meets the [*ProAccessible* requirements](ProAccessible.md) of `F` and `T::is_direct` is `false`, `typename T::template accessor<F>` is inherited by `proxy_indirect_accessor<F>`.

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## See also

[function template `access_proxy`](access_proxy.md)
