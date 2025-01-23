# `std::formatter<proxy_indirect_accessor>::parse()`

```cpp
constexpr typename basic_format_parse_context<CharT>::iterator parse(basic_format_parse_context<CharT>& pc);
```

In the range [`pc.begin()`, `pc.end()`), parses and stores the *format-spec* until the first `}` character.
