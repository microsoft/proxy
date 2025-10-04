# Feature test macro `__msft_lib_proxy`

> Header: `proxy_macros.h` and `proxy.h`

```cpp
#define __msft_lib_proxy /* see below */
```

Starting with 3.0.0, Proxy ships a feature-test macro that encodes the library version. When headers from different major versions of the Proxy library can appear in the same translation unit (for example, Proxy 3 and Proxy 4), use the major-qualified form `__msft_lib_proxy<major>` (e.g., `__msft_lib_proxy4`).

| Version | Value of `__msft_lib_proxy` |
| ------- | --------------------------- |
| 4.0.1   | `202510L`                   |
| 4.0.0   | `202508L`                   |
| 3.4.0   | `202505L`                   |
| 3.3.0   | `202503L`                   |
| 3.2.1   | `202502L`                   |
| 3.2.0   | `202501L`                   |
| 3.1.0   | `202410L`                   |
| 3.0.0   | `202408L`                   |

## Example

```cpp
#include <cstdio>

#include <proxy/proxy.h>

int main() {
#if __msft_lib_proxy >= 202508L
  puts("Compiled with library Proxy 4.0.0 or above.");
#elif __msft_lib_proxy >= 202408L
  puts("Compiled with library Proxy 3.x.");
#else
  puts("Cannot determine the version of library Proxy.");
#endif
}
```
