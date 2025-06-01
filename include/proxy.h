// Compatibility header for Proxy 3.

#ifndef _MSFT_PROXY_3_COMPAT_
#define _MSFT_PROXY_3_COMPAT_

#include "proxy/v3/proxy.h" // IWYU pragma: export

#warning `#include "proxy.h"` is deprecated starting with Proxy 4. \
Use `#include <proxy/proxy.h>` to access the latest Proxy APIs. \
 \
Note: To access the old Proxy 3 APIs, use `#include <proxy/v3/proxy.h>`.

namespace pro { using namespace pro::v3; }

#if defined(PRO_4_DEF_MEM_DISPATCH)
#warning Proxy 3 compatibility header should not be included when a later version of Proxy is also included.
#else
#define PRO_DEF_MEM_DISPATCH(name, ...) PRO_3_DEF_MEM_DISPATCH(name, __VA_ARGS__)
#define PRO_DEF_FREE_DISPATCH(name, ...) PRO_3_DEF_FREE_DISPATCH(name, __VA_ARGS__)
#define PRO_DEF_FREE_AS_MEM_DISPATCH(name, ...) PRO_3_DEF_FREE_AS_MEM_DISPATCH(name, __VA_ARGS__)
#endif

#endif // _MSFT_PROXY_3_COMPAT_
