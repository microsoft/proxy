# Feature test macro `__msft_lib_proxy`

```cpp
#define __msft_lib_proxy /* see below */
```

Similar to the standard [feature test macros](https://en.cppreference.com/w/cpp/feature_test), library "Proxy" has defined a feature test macro since 3.0.0. The table below maps each release version number to the corresponding value of `__msft_lib_proxy`.

| Version | Value of `__msft_lib_proxy` |
| ------- | --------------------------- |
| 3.1.0   | `202410L`                   |
| 3.0.0   | `202408L`                   |

## Example

```cpp
#include <cstdio>

#include "proxy.h"

int main() {
#if defined(__msft_lib_proxy) && __msft_lib_proxy >= 202408L
  puts("Compiled with library Proxy 3.0.0 or above.");
#else
  puts("Cannot determine the version of library Proxy.");
#endif
}
```
