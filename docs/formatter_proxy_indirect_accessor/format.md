# `std::formatter<proxy_indirect_accessor>::format()`

```cpp
template <class OutIt>
OutIt format(const pro::proxy_indirect_accessor<F>& ia, basic_format_context<OutIt, CharT>& fc) const;
```

Formats the managed object `o` represented by `ia` with `std::formatter<std::decay_t<decltype(o)>, CharT>` according to the specifiers stored in `*this`, writes the output to `fc.out()` and returns an end iterator of the output range.
