// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_PROXY_FMT_H_
#define MSFT_PROXY_V4_PROXY_FMT_H_

#include <string_view>
#include <type_traits>

#ifndef __msft_lib_proxy4
#error Please ensure that proxy.h is included before proxy_fmt.h.
#endif // __msft_lib_proxy4

#if FMT_VERSION < 60100
#error Please ensure that the appropriate {fmt} headers (version 6.1.0 or \
later) are included before proxy_fmt.h.
#endif // FMT_VERSION < 60100

namespace pro::inline v4 {

namespace details {

template <class CharT>
#if FMT_VERSION >= 110000
using fmt_buffered_context = fmt::buffered_context<CharT>;
#else
using fmt_buffered_context = fmt::buffer_context<CharT>;
#endif // FMT_VERSION

template <class CharT>
using fmt_format_overload_t = fmt_buffered_context<CharT>::iterator(
    std::basic_string_view<CharT> spec, fmt_buffered_context<CharT>& fc) const;

struct fmt_format_dispatch {
  template <class T, class CharT, class FormatContext>
  PRO4D_STATIC_CALL(auto, const T& self, std::basic_string_view<CharT> spec,
                    FormatContext& fc)
    requires(std::is_default_constructible_v<fmt::formatter<T, CharT>>)
  {
    fmt::formatter<T, CharT> impl;
    {
      fmt::basic_format_parse_context<CharT> pc{spec};
      impl.parse(pc);
    }
    return impl.format(self, fc);
  }
};

} // namespace details

namespace skills {

template <class FB>
using fmt_format =
    typename FB::template add_convention<details::fmt_format_dispatch,
                                         details::fmt_format_overload_t<char>>;

template <class FB>
using fmt_wformat = typename FB::template add_convention<
    details::fmt_format_dispatch, details::fmt_format_overload_t<wchar_t>>;

} // namespace skills

} // namespace pro::inline v4

namespace fmt {

template <pro::v4::facade F, class CharT>
  requires(pro::v4::details::facade_traits<F>::template is_invocable<
           false, pro::v4::details::fmt_format_dispatch,
           pro::v4::details::fmt_format_overload_t<CharT>>)
struct formatter<pro::v4::proxy_indirect_accessor<F>, CharT> {
  constexpr auto parse(basic_format_parse_context<CharT>& pc) {
    for (auto it = pc.begin(); it != pc.end(); ++it) {
      if (*it == '}') {
        spec_ = std::basic_string_view<CharT>{pc.begin(), it + 1};
        return it;
      }
    }
    return pc.end();
  }

  template <class FormatContext>
  auto format(const pro::v4::proxy_indirect_accessor<F>& p,
              FormatContext& fc) const -> typename FormatContext::iterator {
    return pro::v4::proxy_invoke<
        pro::v4::details::fmt_format_dispatch,
        pro::v4::details::fmt_format_overload_t<CharT>>(p, spec_, fc);
  }

private:
  std::basic_string_view<CharT> spec_;
};

} // namespace fmt

#endif // MSFT_PROXY_V4_PROXY_FMT_H_
