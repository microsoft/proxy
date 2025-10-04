module;

#include <proxy/v4/proxy.h>

export module proxy.v4;

export namespace pro::inline v4 {

using v4::allocate_proxy;
using v4::allocate_proxy_shared;
using v4::bad_proxy_cast;
using v4::basic_facade_builder;
using v4::constraint_level;
using v4::conversion_dispatch;
using v4::explicit_conversion_dispatch;
using v4::facade;
using v4::facade_aware_overload_t;
using v4::facade_builder;
using v4::implicit_conversion_dispatch;
using v4::inplace_proxiable_target;
using v4::is_bitwise_trivially_relocatable;
using v4::is_bitwise_trivially_relocatable_v;
using v4::make_proxy;
using v4::make_proxy_inplace;
using v4::make_proxy_shared;
using v4::make_proxy_view;
using v4::not_implemented;
using v4::observer_facade;
using v4::operator_dispatch;
using v4::proxiable;
using v4::proxiable_target;
using v4::proxy;
using v4::proxy_indirect_accessor;
using v4::proxy_invoke;
using v4::proxy_reflect;
using v4::proxy_view;
using v4::substitution_dispatch;
using v4::weak_dispatch;
using v4::weak_facade;
using v4::weak_proxy;

namespace skills {

#if __STDC_HOSTED__ && __has_include(<format>)
using skills::format;
using skills::wformat;
#endif // __STDC_HOSTED__ && __has_include(<format>)

#if __cpp_rtti >= 199711L
using skills::direct_rtti;
using skills::indirect_rtti;
using skills::rtti;
#endif // __cpp_rtti >= 199711L

using skills::as_view;
using skills::as_weak;
using skills::slim;

} // namespace skills

} // namespace pro::inline v4

#if __STDC_HOSTED__ && __has_include(<format>)
export namespace std {

using std::formatter;

} // namespace std
#endif // __STDC_HOSTED__ && __has_include(<format>)
