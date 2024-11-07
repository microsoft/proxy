# Proxy 3 Specifications

This document provides the API specifications for the C++ library Proxy (version 3). All the documented concepts, classes, and functions are defined in the namespace `pro`. Unless otherwise specified, all facilities are [freestanding](https://en.cppreference.com/w/cpp/freestanding) by default.

## Concepts

| Name                                                      | Description                                                  |
| --------------------------------------------------------- | ------------------------------------------------------------ |
| [`facade`](facade.md)                                     | Specifies that a type models a "facade"                      |
| [`proxiable`](proxiable.md)                               | Specifies that a pointer type can instantiate a `proxy`      |
| [`inplace_proxiable_target`](inplace_proxiable_target.md) | Specifies that a value type can instantiate a `proxy` without allocation |

## Classes

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`proxy`](proxy.md)                                          | Wraps a pointer object matching specified facade             |
| [`basic_facade_builder`<br />`facade_builder`](basic_facade_builder.md) | Provides capability to build a facade type at compile-time   |
| [`constraint_level`](constraint_level.md)                    | Defines the 4 constraint levels of a special member function |
| [`proxiable_ptr_constraints`](proxiable_ptr_constraints.md)  | Defines the constraints of a pointer type to instantiate a `proxy` |
| [`operator_dispatch`](operator_dispatch.md)                  | Dispatch type for operator expressions with accessibility    |
| [`conversion_dispatch`](conversion_dispatch.md)              | Dispatch type for conversion expressions with accessibility  |

## Functions

| Name                                          | Description                                                  |
| --------------------------------------------- | ------------------------------------------------------------ |
| [`make_proxy`](make_proxy.md)                 | Creates a `proxy` object potentially with heap allocation    |
| [`make_proxy_inplace`](make_proxy_inplace.md) | Creates a `proxy` object with strong no-allocation guarantee |
| [`allocate_proxy`](allocate_proxy.md)         | Creates a `proxy` object with an allocator                   |
| [`proxy_invoke`](proxy_invoke.md)             | Invokes a `proxy` with a specified convention                |
| [`proxy_reflect`](proxy_reflect.md)           | Acquires reflection information of the underlying pointer type |
| [`access_proxy`](access_proxy.md)             | Accesses a `proxy` object via an accessor                    |

## Macros

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)            | Defines a dispatch type for member function call expressions with accessibility |
| [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md)          | Defines a dispatch type for free function call expressions with accessibility |
| [`PRO_DEF_FREE_AS_MEM_DISPATCH`](PRO_DEF_FREE_AS_MEM_DISPATCH.md) | Defines a dispatch type for free function call expressions with accessibility via a member function |
| [`PRO_DEF_WEAK_DISPATCH`](PRO_DEF_WEAK_DISPATCH.md)          | Defines a weak dispatch type with a default implementation   |
| [`__msft_lib_proxy`](msft_lib_proxy.md)                      | Feature test macro                                           |

## Named Requirements

| Name                                          | Description                                                  |
| --------------------------------------------- | ------------------------------------------------------------ |
| [*ProBasicFacade*](ProBasicFacade.md)         | Specifies that a type potentially models a "facade" of `proxy` |
| [*ProBasicConvention*](ProBasicConvention.md) | Specifies that a type potentially models a "convention"      |
| [*ProBasicReflection*](ProBasicReflection.md) | Specifies that a type potentially models a "reflection"      |
| [*ProFacade*](ProFacade.md)                   | Specifies that a type models a "facade" of `proxy`           |
| [*ProConvention*](ProConvention.md)           | Specifies that a type models a "convention"                  |
| [*ProReflection*](ProReflection.md)           | Specifies that a type models a "reflection"                  |
| [*ProDispatch*](ProDispatch.md)               | Specifies that a type models a "dispatch"                    |
| [*ProOverload*](ProOverload.md)               | Specifies that a type models an "overload"                   |
| [*ProAccessible*](ProAccessible.md)           | Specifies that a type provides accessibility to `proxy`      |
