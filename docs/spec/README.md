# Proxy 4 Specifications

This document provides the API specifications for the C++ library Proxy (version 4). All the documented concepts, classes, and functions are defined in the namespace `pro::inline v4`. Unless otherwise specified, all facilities are [freestanding](https://en.cppreference.com/w/cpp/freestanding) by default.

## Header `<proxy.h>`

### Concepts

| Name                                                      | Description                                                  |
| --------------------------------------------------------- | ------------------------------------------------------------ |
| [`facade`](facade.md)                                     | Specifies that a type models a "facade"                      |
| [`inplace_proxiable_target`](inplace_proxiable_target.md) | Specifies that a value type can instantiate a `proxy` without allocation |
| [`proxiable_target`](proxiable_target.md)                 | Specifies that a reference type can instantiate a `proxy_view` |
| [`proxiable`](proxiable.md)                               | Specifies that a pointer type can instantiate a `proxy`      |

### Classes

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`bad_proxy_cast`](bad_proxy_cast.md)                        | Exception thrown by the value-returning forms of `proxy_cast` on a type mismatch |
| [`basic_facade_builder`<br />`facade_builder`](basic_facade_builder/README.md) | Provides capability to build a facade type at compile-time   |
| [`constraint_level`](constraint_level.md)                    | Defines the 4 constraint levels of a special member function |
| [`explicit_conversion_dispatch`<br />`conversion_dispatch`](explicit_conversion_dispatch/README.md) | Dispatch type for explicit conversion expressions with accessibility |
| [`facade_aware_overload_t`](facade_aware_overload_t.md)      | Specifies a facade-aware overload template                   |
| [`implicit_conversion_dispatch`](implicit_conversion_dispatch/README.md) | Dispatch type for implicit conversion expressions with accessibility |
| [`not_implemented` ](not_implemented.md)                     | Exception thrown by `weak_dispatch` for the default implementation |
| [`operator_dispatch`](operator_dispatch/README.md)           | Dispatch type for operator expressions with accessibility    |
| [`proxiable_ptr_constraints`](proxiable_ptr_constraints.md)  | Defines the constraints of a pointer type to instantiate a `proxy` |
| [`proxy_indirect_accessor`](proxy_indirect_accessor.md)      | Provides indirection accessibility for `proxy`               |
| [`proxy_view`<br />`observer_facade`](proxy_view.md)         | Non-owning `proxy` optimized for raw pointer types           |
| [`proxy`](proxy/README.md)                                   | Wraps a pointer object matching specified facade             |
| [`weak_dispatch`](weak_dispatch/README.md)                   | Weak dispatch type with a default implementation that throws `not_implemented` |
| [`weak_proxy`<br />`weak_facade`](weak_proxy.md)             | `proxy` with weak ownership                                  |

### Alias Templates

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`skills::as_view`](skills_as_view.md)                       | `facade` skill set: implicit conversion to `proxy_view`      |
| [`skills::as_weak`](skills_as_weak.md)                       | `facade` skill set: implicit conversion to `weak_proxy`      |
| [`skills::format`<br />`skills::wformat`](skills_format.md)  | `facade` skill set: formatting via the [standard formatting functions](https://en.cppreference.com/w/cpp/utility/format) |
| [`skills::rtti`<br />`skills::indirect_rtti`<br />`skills::direct_rtti` ](skills_rtti/README.md) | `facade` skill set: RTTI via `proxy_cast` and `proxy_typeid` |

### Functions

| Name                                                | Description                                                  |
| --------------------------------------------------- | ------------------------------------------------------------ |
| [`access_proxy`](access_proxy.md)                   | Accesses a `proxy` object via an accessor                    |
| [`allocate_proxy_shared`](allocate_proxy_shared.md) | Creates a `proxy` object with shared ownership using an allocator |
| [`allocate_proxy`](allocate_proxy.md)               | Creates a `proxy` object with an allocator                   |
| [`make_proxy_inplace`](make_proxy_inplace.md)       | Creates a `proxy` object with strong no-allocation guarantee |
| [`make_proxy_shared`](make_proxy_shared.md)         | Creates a `proxy` object with shared ownership               |
| [`make_proxy_view`](make_proxy_view.md)             | Creates a `proxy_view` object                                |
| [`make_proxy`](make_proxy.md)                       | Creates a `proxy` object potentially with heap allocation    |
| [`proxy_invoke`](proxy_invoke.md)                   | Invokes a `proxy` with a specified convention                |
| [`proxy_reflect`](proxy_reflect.md)                 | Acquires reflection information of the underlying pointer type |

## Header `<proxy_macros.h>`

Also included in `proxy.h`.

### Macros

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`__msft_lib_proxy`](msft_lib_proxy.md)                      | Feature test macro                                           |
| [`PRO_DEF_FREE_AS_MEM_DISPATCH` ](PRO_DEF_FREE_AS_MEM_DISPATCH.md) | Defines a dispatch type for free function call expressions with accessibility via a member function |
| [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md)          | Defines a dispatch type for free function call expressions with accessibility |
| [`PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)            | Defines a dispatch type for member function call expressions with accessibility |

## Header `<proxy_fmt.h>`

### Alias Templates

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`skills::fmt_format`<br />`skills::fmt_wformat`](skills_fmt_format.md) | `facade` skill set: formatting via the [{fmt} library](https://github.com/fmtlib/fmt) |

## Named Requirements

| Name                                          | Description                                                  |
| --------------------------------------------- | ------------------------------------------------------------ |
| [*ProAccessible*](ProAccessible.md)           | Specifies that a type provides accessibility to `proxy`      |
| [*ProBasicConvention*](ProBasicConvention.md) | Specifies that a type potentially models a "convention"      |
| [*ProBasicFacade*](ProBasicFacade.md)         | Specifies that a type potentially models a "facade" of `proxy` |
| [*ProBasicReflection*](ProBasicReflection.md) | Specifies that a type potentially models a "reflection"      |
| [*ProConvention*](ProConvention.md)           | Specifies that a type models a "convention"                  |
| [*ProDispatch*](ProDispatch.md)               | Specifies that a type models a "dispatch"                    |
| [*ProFacade*](ProFacade.md)                   | Specifies that a type models a "facade" of `proxy`           |
| [*ProOverload*](ProOverload.md)               | Specifies that a type models an "overload"                   |
| [*ProReflection*](ProReflection.md)           | Specifies that a type models a "reflection"                  |
