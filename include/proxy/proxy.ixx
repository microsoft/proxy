module;

#include <proxy/v4/proxy.h>

export module proxy.v4;

export namespace pro {

inline namespace v4 {

using pro::constraint_level;
using pro::proxiable_ptr_constraints;
using pro::facade_aware_overload_t;
using pro::facade;
using pro::proxy_indirect_accessor;
using pro::proxy;
using pro::proxiable;
using pro::proxy_indirect_accessor;
using pro::proxy_invoke;
using pro::proxy_reflect;
using pro::access_proxy;
using pro::observer_facade;
using pro::proxy_view;
using pro::weak_proxy;
using pro::inplace_proxiable_target;
using pro::proxiable_target;
using pro::make_proxy_inplace;
using pro::make_proxy_view;
using pro::allocate_proxy;
using pro::make_proxy;
using pro::allocate_proxy_shared;
using pro::make_proxy_shared;
using pro::bad_proxy_cast;
using pro::observer_facade;
using pro::weak_facade;
using pro::basic_facade_builder;
using pro::operator_dispatch;
using pro::implicit_conversion_dispatch;
using pro::explicit_conversion_dispatch;
using pro::conversion_dispatch;
using pro::not_implemented;
using pro::weak_dispatch;
using pro::facade_builder;

namespace skills {
using skills::direct_rtti;
using skills::indirect_rtti;
} // namespace pro::skills

// Currently, these are required by PRO_DEF_... macros.
// In the future the macros might be refactored to avoid depending
// on implementation details.
namespace details {
using details::adl_accessor_arg_t;
using details::non_proxy_arg;
} // namespace pro::details

} // namespace v4

} // namespace pro

export namespace std {
using std::formatter;
}
