module;

#include <proxy/v4/proxy.h>

export module proxy.v4;

export namespace pro {

inline namespace v4 {

using v4::constraint_level;
using v4::proxiable_ptr_constraints;
using v4::facade_aware_overload_t;
using v4::facade;
using v4::proxy_indirect_accessor;
using v4::proxy;
using v4::proxiable;
using v4::proxy_indirect_accessor;
using v4::proxy_invoke;
using v4::proxy_reflect;
using v4::access_proxy;
using v4::observer_facade;
using v4::proxy_view;
using v4::weak_proxy;
using v4::inplace_proxiable_target;
using v4::proxiable_target;
using v4::make_proxy_inplace;
using v4::make_proxy_view;
using v4::allocate_proxy;
using v4::make_proxy;
using v4::allocate_proxy_shared;
using v4::make_proxy_shared;
using v4::bad_proxy_cast;
using v4::observer_facade;
using v4::weak_facade;
using v4::basic_facade_builder;
using v4::operator_dispatch;
using v4::implicit_conversion_dispatch;
using v4::explicit_conversion_dispatch;
using v4::conversion_dispatch;
using v4::not_implemented;
using v4::weak_dispatch;
using v4::facade_builder;

namespace skills {
using skills::direct_rtti;
using skills::indirect_rtti;
} // namespace v4::skills

// Currently, these are required by PRO_DEF_... macros.
// In the future the macros might be refactored to avoid depending
// on implementation details.
namespace details {
using details::adl_accessor_arg_t;
using details::non_proxy_arg;
} // namespace v4::details

} // namespace v4

} // namespace pro

export namespace std {
using std::formatter;
}
