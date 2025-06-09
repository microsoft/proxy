# Class `bad_proxy_cast`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
class bad_proxy_cast : public std::bad_cast;
```

A type of object to be thrown by the value-returning forms of [`proxy_cast`](skills_rtti/proxy_cast.md) on failure.

## Member Functions

| Name          | Description                          |
| ------------- | ------------------------------------ |
| (constructor) | constructs a `bad_proxy_cast` object |
| (destructor)  | destroys a `bad_proxy_cast` object   |
| `what`        | returns the explanatory string       |
