# Class template `operator_dispatch`

The definition of `operator_dispatch` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Sign, bool Rhs = false>
class operator_dispatch;
```

Class template `operator_dispatch` is a [dispatch](ProDispatch.md) type for operator expressions. `Sign` represents the sign of operator (SOP) as a string literal (e.g., `"+"` for operator `+`). `Rhs` specifies whether the [`proxy`](proxy.md) operand is on the right-hand side of a binary operator.

## Supported SOPs

As per C++20 specifications, there are 44 different SOPs. `operator_dispatch` supports the following 36 SOPs:

```text
"+", "-", "*", "/", "%", "++", "--", "==", "!=", ">", "<", ">=", "<=", "<=>", "!", "&&", "||", "~", "&", "|", "^", "<<", ">>", "+=", "-=", "*=", "/=", "&=", "|=", "^=", "<<=", ">>=", ",", "->*", "()", "[]"
```

The following 8 SOPs are not supported:

- `new`, `new[]`, `delete`, `delete[]`, and `co_await` are context-dependent and not applicable for `proxy`.
- Assignment (`=`) and indirection (`->`) are ambiguous in the context of `proxy`.
- Literals (`""`) apply only to a limited number of primitive types and are not applicable for `proxy`.

## Type Conversion

Type conversion expressions, although using `operator` syntax, do not have a specific sign and are not within the scope of `operator_dispatch`. They are supported by another class template [`conversion_dispatch`](conversion_dispatch.md).

## Specializations

Let `self` be the operand of [`proxy`](proxy.md), and `other` and `others...` be the other operand(s) in the expression of an operator. `operator_dispatch` has the following specializations for various expressions:

| Specializations                   | Expressions             |
| --------------------------------- | ----------------------- |
| `operator_dispatch<"+", false>`   | `+self`, `self + other` |
| `operator_dispatch<"-", false>`   | `-self`, `self - other` |
| `operator_dispatch<"*", false>`   | `*self`, `self * other` |
| `operator_dispatch<"/", false>`   | `self / other`          |
| `operator_dispatch<"%", false>`   | `self % other`          |
| `operator_dispatch<"++", false>`  | `++self`, `self++`      |
| `operator_dispatch<"--", false>`  | `--self`, `self--`      |
| `operator_dispatch<"==", false>`  | `self == other`         |
| `operator_dispatch<"!=", false>`  | `self != other`         |
| `operator_dispatch<">", false>`   | `self > other`          |
| `operator_dispatch<"<", false>`   | `self < other`          |
| `operator_dispatch<">=", false>`  | `self >= other`         |
| `operator_dispatch<"<=", false>`  | `self <= other`         |
| `operator_dispatch<"<=>", false>` | `self <=> other`        |
| `operator_dispatch<"!", false>`   | `!self`                 |
| `operator_dispatch<"&&", false>`  | `self && other`         |
| `operator_dispatch<"||", false>`  | `self || other`         |
| `operator_dispatch<"~", false>`   | `!self`                 |
| `operator_dispatch<"&", false>`   | `&self`, `self & other` |
| `operator_dispatch<"|", false>`   | `self | other`          |
| `operator_dispatch<"^", false>`   | `self ^ other`          |
| `operator_dispatch<"<<", false>`  | `self << other`         |
| `operator_dispatch<">>", false>`  | `self >> other`         |
| `operator_dispatch<"+=", false>`  | `self += other`         |
| `operator_dispatch<"-=", false>`  | `self -= other`         |
| `operator_dispatch<"*=", false>`  | `self *= other`         |
| `operator_dispatch<"/=", false>`  | `self /= other`         |
| `operator_dispatch<"&=", false>`  | `self &= other`         |
| `operator_dispatch<"|=", false>`  | `self |= other`         |
| `operator_dispatch<"^=", false>`  | `self ^= other`         |
| `operator_dispatch<"<<=", false>` | `self <<= other`        |
| `operator_dispatch<">>=", false>` | `self >>= other`        |
| `operator_dispatch<",", false>`   | `self, other`           |
| `operator_dispatch<"->*", false>` | `self ->* other`        |
| `operator_dispatch<"()", false>`  | `self(others...)`       |
| `operator_dispatch<"[]", false>`  | `self[other]` (until C++23)<br />`self[others...]` (since C++23) |
| `operator_dispatch<"+", true>`    | `other + self`          |
| `operator_dispatch<"-", true>`    | `other - self`          |
| `operator_dispatch<"*", true>`    | `other * self`          |
| `operator_dispatch<"/", true>`    | `other / self`          |
| `operator_dispatch<"%", true>`    | `other % self`          |
| `operator_dispatch<"==", true>`   | `other == self`         |
| `operator_dispatch<"!=", true>`   | `other != self`         |
| `operator_dispatch<">", true>`    | `other > self`          |
| `operator_dispatch<"<", true>`    | `other < self`          |
| `operator_dispatch<">=", true>`   | `other >= self`         |
| `operator_dispatch<"<=", true>`   | `other <= self`         |
| `operator_dispatch<"<=>", true>`  | `other <=> self`        |
| `operator_dispatch<"&&", true>`   | `other && self`         |
| `operator_dispatch<"||", true>`   | `other || self`         |
| `operator_dispatch<"&", true>`    | `other & self`          |
| `operator_dispatch<"|", true>`    | `other | self`          |
| `operator_dispatch<"^", true>`    | `other ^ self`          |
| `operator_dispatch<"<<", true>`   | `other << self`         |
| `operator_dispatch<">>", true>`   | `other >> self`         |
| `operator_dispatch<"+=", true>`   | `other += self`         |
| `operator_dispatch<"-=", true>`   | `other -= self`         |
| `operator_dispatch<"*=", true>`   | `other *= self`         |
| `operator_dispatch<"/=", true>`   | `other /= self`         |
| `operator_dispatch<"&=", true>`   | `other &= self`         |
| `operator_dispatch<"|=", true>`   | `other |= self`         |
| `operator_dispatch<"^=", true>`   | `other ^= self`         |
| `operator_dispatch<"<<=", true>`  | `other <<= self`        |
| `operator_dispatch<">>=", true>`  | `other >>= self`        |
| `operator_dispatch<",", true>`    | `other, self`           |
| `operator_dispatch<"->*", true>`  | `other ->* self`        |

## Member Functions

| Name                                               | Description                              |
| -------------------------------------------------- | ---------------------------------------- |
| (constructor) [trivial]                            | constructs an `operator_dispatch` object |
| [`operator()`](operator_dispatch/operator_call.md) | invokes the dispatch                     |

## Member Types

| Name                                        | Description                       |
| ------------------------------------------- | --------------------------------- |
| [`accessor`](operator_dispatch/accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iomanip>
#include <iostream>
#include <numbers>

#include "proxy.h"

struct Number : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"*=">, void(int)>
    ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream&) const&>
    ::build {};

int main() {
  pro::proxy<Number> p1 = pro::make_proxy<Number>(std::numbers::pi);
  *p1 *= 3;
  std::cout << std::setprecision(10) << *p1 << "\n";  // Prints: 9.424777961

  pro::proxy<Number> p2 = pro::make_proxy<Number>(10);
  *p2 *= 5;
  std::cout << *p2 << "\n";  // Prints: 50
}
```

## See Also

- [class template `conversion_dispatch`](conversion_dispatch.md)
