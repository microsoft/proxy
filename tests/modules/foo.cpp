module;

#include <proxy/proxy_macros.h>

#include <type_traits>
#include <utility>

export module foo;

import proxy.v4;

extern "C++" {
PRO_DEF_MEM_DISPATCH(MemGetFoo, GetFoo);
PRO_DEF_FREE_DISPATCH(FreeGetBar, GetBar);
PRO_DEF_FREE_AS_MEM_DISPATCH(FreeGetBaz, GetBaz);
}

export struct Foo
    : pro::facade_builder::add_convention<MemGetFoo, int() const>::build {};

export struct Bar
    : pro::facade_builder::add_convention<FreeGetBar, int() const>::build {};

export struct Baz
    : pro::facade_builder::add_convention<FreeGetBaz, int() const>::build {};
