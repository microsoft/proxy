# Class `bad_proxy_cast`

```cpp
class bad_proxy_cast : public std::bad_cast;
```

A type of object to be thrown by the value-returning forms of [`proxy_cast`](basic_facade_builder/support_rtti.md) on failure.

## Member Functions

| Name          | Description                          |
| ------------- | ------------------------------------ |
| (constructor) | constructs a `bad_proxy_cast` object |
| (destructor)  | destroys a `bad_proxy_cast` object   |
| `what`        | returns the explanatory string       |
