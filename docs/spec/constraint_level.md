# Enum class `constraint_level`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

```cpp
enum class constraint_level : /* unspecified */ {
  none, nontrivial, nothrow, trivial
};
```

`constraint_level` defines the constraints on a type's lifetime operations (construction, relocation, or destruction). For a given operation `O`:

- `none`: No restrictions on `O`.
- `nontrivial`: `O` shall be supported.
- `nothrow`: `O` shall be supported and shall not throw.
- `trivial`: `O` shall be supported and shall be trivial.

The specific semantics of each value depends on its context.

## See Also

- [concept `facade`](facade.md)
