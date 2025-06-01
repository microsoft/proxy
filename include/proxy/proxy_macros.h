#include "v4/proxy_macros.h" // IWYU pragma: export

#ifdef PRO_DEF_MEM_DISPATCH
#undef PRO_DEF_MEM_DISPATCH
#endif
#define PRO_DEF_MEM_DISPATCH(name, ...) PRO_4_DEF_MEM_DISPATCH(name, __VA_ARGS__)

#ifdef PRO_DEF_FREE_DISPATCH
#undef PRO_DEF_FREE_DISPATCH
#endif
#define PRO_DEF_FREE_DISPATCH(name, ...) PRO_4_DEF_FREE_DISPATCH(name, __VA_ARGS__)

#ifdef PRO_DEF_FREE_AS_MEM_DISPATCH
#undef PRO_DEF_FREE_AS_MEM_DISPATCH
#endif
#define PRO_DEF_FREE_AS_MEM_DISPATCH(name, ...) PRO_4_DEF_FREE_AS_MEM_DISPATCH(name, __VA_ARGS__)
