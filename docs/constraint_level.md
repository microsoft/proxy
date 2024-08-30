# Enum class `constraint_level`

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

- [class `proxiable_ptr_constraints`](proxiable_ptr_constraints.md)
