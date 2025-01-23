# Proxy 3 Specifications

This document provides the API specifications for the C++ library Proxy (version 3). All the documented concepts, classes, and functions are defined in the namespace `pro`. Unless otherwise specified, all facilities are [freestanding](https://en.cppreference.com/w/cpp/freestanding) by default.

## Concepts

| Name                                                      | Description                                                  |
| --------------------------------------------------------- | ------------------------------------------------------------ |
| [`facade`](facade.md)                                     | Specifies that a type models a "facade"                      |
| [`inplace_proxiable_target`](inplace_proxiable_target.md) | Specifies that a value type can instantiate a `proxy` without allocation |
| [`proxiable`](proxiable.md)                               | Specifies that a pointer type can instantiate a `proxy`      |

## Classes

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`bad_proxy_cast`](bad_proxy_cast.md)<br />*(since 3.2)*     | Exception thrown by the value-returning forms of `proxy_cast` on a type mismatch |
| [`basic_facade_builder`<br />`facade_builder`](basic_facade_builder.md) | Provides capability to build a facade type at compile-time   |
| [`constraint_level`](constraint_level.md)                    | Defines the 4 constraint levels of a special member function |
| [`explicit_conversion_dispatch`<br />`conversion_dispatch`](explicit_conversion_dispatch.md)<br />*(since 3.2)* | Dispatch type for explicit conversion expressions with accessibility |
| [`implicit_conversion_dispatch`](implicit_conversion_dispatch.md)<br />*(since 3.2)* | Dispatch type for implicit conversion expressions with accessibility |
| [`not_implemented` ](not_implemented.md)<br />*(since 3.2)*  | Exception thrown by `weak_dispatch` for the default implementation |
| [`observer_facade`<br />`proxy_view`](observer_facade.md)<br />*(since 3.2)* | Non-owning proxy optimized for raw pointers                  |
| [`operator_dispatch`](operator_dispatch.md)                  | Dispatch type for operator expressions with accessibility    |
| [`proxiable_ptr_constraints`](proxiable_ptr_constraints.md)  | Defines the constraints of a pointer type to instantiate a `proxy` |
| [`proxy_indirect_accessor`](proxy_indirect_accessor.md)<br />*(since 3.2)* | Provides indirection accessibility for `proxy`               |
| [`proxy`](proxy.md)                                          | Wraps a pointer object matching specified facade             |
| [`std::formatter<proxy_indirect_accessor>`](formatter_proxy_indirect_accessor.md)<br />*(since 3.2)* | Formatting support for `proxy_indirect_accessor`             |
| [`weak_dispatch`](weak_dispatch.md)<br />*(since 3.2)*       | Weak dispatch type with a default implementation that throws `not_implemented` |

## Functions

| Name                                          | Description                                                  |
| --------------------------------------------- | ------------------------------------------------------------ |
| [`access_proxy`](access_proxy.md)             | Accesses a `proxy` object via an accessor                    |
| [`allocate_proxy`](allocate_proxy.md)         | Creates a `proxy` object with an allocator                   |
| [`make_proxy_inplace`](make_proxy_inplace.md) | Creates a `proxy` object with strong no-allocation guarantee |
| [`make_proxy`](make_proxy.md)                 | Creates a `proxy` object potentially with heap allocation    |
| [`proxy_invoke`](proxy_invoke.md)             | Invokes a `proxy` with a specified convention                |
| [`proxy_reflect`](proxy_reflect.md)           | Acquires reflection information of the underlying pointer type |

## Macros

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`__msft_lib_proxy`](msft_lib_proxy.md)                      | Feature test macro                                           |
| [`PRO_DEF_FREE_AS_MEM_DISPATCH` ](PRO_DEF_FREE_AS_MEM_DISPATCH.md)<br />*(since 3.1)* | Defines a dispatch type for free function call expressions with accessibility via a member function |
| [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md)          | Defines a dispatch type for free function call expressions with accessibility |
| [`PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)            | Defines a dispatch type for member function call expressions with accessibility |
| [`PRO_DEF_WEAK_DISPATCH`](PRO_DEF_WEAK_DISPATCH.md)<br />*(deprecated since 3.2)* | Defines a weak dispatch type with a default implementation   |

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
