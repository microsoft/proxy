# `std::formatter<proxy_indirect_accessor>`

```cpp
namespace std {
  template <class F, class CharT> requires(/* see below */)
  struct formatter<pro::proxy_indirect_accessor<F>, ChatT>;
}
```

The template specialization of [`std::formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter) for [`proxy_indirect_accessor`](proxy_indirect_accessor.md) allows users to convert an object managed by [`proxy`](proxy.md) to string using [formatting functions](https://en.cppreference.com/w/cpp/utility/format) such as [`std::format`](https://en.cppreference.com/w/cpp/utility/format/format). The specialization is enabled when `F` is built from [`basic_facade_builder`](basic_facade_builder.md) and

- when `CharT` is `char`, specified [`support_format`](basic_facade_builder/support_format.md), or
- when `CharT` is `wchar_t`, specified [`support_wformat`](basic_facade_builder/support_format.md).

## Member Functions

| Name                                                         | Description                         |
| ------------------------------------------------------------ | ----------------------------------- |
| [`parse`](formatter_proxy_indirect_accessor/parse.md) [constexpr] | parse the format spec               |
| [`format`](formatter_proxy_indirect_accessor/format.md)      | format according to the format spec |

## See Also

- [`basic_facade_builder::support_format`](basic_facade_builder/support_format)
