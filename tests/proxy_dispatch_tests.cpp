// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <vector>
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable: 4834)  // False alarm from MSVC: warning C4834: discarding return value of function with [[nodiscard]] attribute
#endif  // defined(_MSC_VER) && !defined(__clang__)
#include "proxy.h"
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif  // defined(_MSC_VER) && !defined(__clang__)

namespace proxy_dispatch_tests_details {

struct CommaTester {
public:
  explicit CommaTester(int v) : value_(v) {}
  int operator,(int v) { return value_ + v; }
  friend int operator,(int v, CommaTester self) { return v * self.value_; }

private:
  int value_;
};

struct PtrToMemTester {
public:
  explicit PtrToMemTester(int v) : value_(v) {}
  friend int operator->*(int v, PtrToMemTester self) { return v * self.value_; }

private:
  int value_;
};

PRO_DEF_FREE_AS_MEM_DISPATCH(FreeMemToString, std::to_string, ToString);

}  // namespace proxy_dispatch_tests_details

namespace details = proxy_dispatch_tests_details;

TEST(ProxyDispatchTests, TestOpPlus) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"+">, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(+*p, 12);
  ASSERT_EQ(*p + 2, 14);
}

TEST(ProxyDispatchTests, TestOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"-">, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(-*p, -12);
  ASSERT_EQ(*p - 2, 10);
}

TEST(ProxyDispatchTests, TestOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"*">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p * 2, 24);
}

TEST(ProxyDispatchTests, TestOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"/">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p / 2, 6);
}

TEST(ProxyDispatchTests, TestOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"%">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p % 5, 2);
}

TEST(ProxyDispatchTests, TestOpIncrement) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"++">, int(), int(int)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(++(*p), 13);
  ASSERT_EQ((*p)++, 13);
  ASSERT_EQ(v, 14);
}

TEST(ProxyDispatchTests, TestOpDecrement) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"--">, int(), int(int)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(--(*p), 11);
  ASSERT_EQ((*p)--, 11);
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"==">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p == 12, true);
  ASSERT_EQ(*p == 11, false);
}

TEST(ProxyDispatchTests, TestOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"!=">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p != 12, false);
  ASSERT_EQ(*p != 11, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p > 2, true);
  ASSERT_EQ(*p > 20, false);
}

TEST(ProxyDispatchTests, TestOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p < 2, false);
  ASSERT_EQ(*p < 20, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">=">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p >= 20, false);
  ASSERT_EQ(*p >= 12, true);
}

TEST(ProxyDispatchTests, TestOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<=">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p <= 2, false);
  ASSERT_EQ(*p <= 12, true);
}

TEST(ProxyDispatchTests, TestOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<=>">, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p <=> 2, std::strong_ordering::greater);
  ASSERT_EQ(*p <=> 12, std::strong_ordering::equal);
  ASSERT_EQ(*p <=> 20, std::strong_ordering::less);
}

TEST(ProxyDispatchTests, TestOpLogicalNot) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"!">, bool()>::build {};
  int v1 = 12, v2 = 0;
  pro::proxy<TestFacade> p1 = &v1, p2 = &v2;
  ASSERT_EQ(!*p1, false);
  ASSERT_EQ(!*p2, true);
}

TEST(ProxyDispatchTests, TestOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&&">, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p && true, true);
  ASSERT_EQ(*p && false, false);
}

TEST(ProxyDispatchTests, TestOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"||">, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p || true, true);
  ASSERT_EQ(*p || false, false);
}

TEST(ProxyDispatchTests, TestOpTilde) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"~">, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(~*p, -13);
}

TEST(ProxyDispatchTests, TestOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&">, const void* () noexcept, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(&*p, &v);
  ASSERT_EQ(*p & 4, 4);
}

TEST(ProxyDispatchTests, TestOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"|">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p | 6, 14);
}

TEST(ProxyDispatchTests, TestOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"^">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p ^ 5, 9);
}

TEST(ProxyDispatchTests, TestOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<<">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p << 2, 48);
}

TEST(ProxyDispatchTests, TestOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">>">, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p >> 2, 3);
}

TEST(ProxyDispatchTests, TestOpPlusAssignment) {
  struct TestFacade : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"+=">, void(int val)>
      ::add_direct_convention<pro::operator_dispatch<"+=">, void(int val)>
      ::build {};
  int v[3] = {12, 0, 7};
  pro::proxy<TestFacade> p = v;
  (*p += 2) += 3;
  p += 2;
  *p += 100;
  ASSERT_EQ(v[0], 17);
  ASSERT_EQ(v[1], 0);
  ASSERT_EQ(v[2], 107);
}

TEST(ProxyDispatchTests, TestOpMinusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"-=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p -= 2) -= 3;
  ASSERT_EQ(v, 7);
}

TEST(ProxyDispatchTests, TestOpMultiplicationAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"*=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p *= 2) *= 3;
  ASSERT_EQ(v, 72);
}

TEST(ProxyDispatchTests, TestOpDivisionAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"/=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p /= 2) /= 2;
  ASSERT_EQ(v, 3);
}

TEST(ProxyDispatchTests, TestOpBitwiseAndAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&=">, void(int val)>::build {};
  int v = 15;
  pro::proxy<TestFacade> p = &v;
  (*p &= 11) &= 14;
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpBitwiseOrAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"|=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p |= 2) |= 1;
  ASSERT_EQ(v, 15);
}

TEST(ProxyDispatchTests, TestOpBitwiseXorAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"^=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p ^= 6) ^= 1;
  ASSERT_EQ(v, 11);
}

TEST(ProxyDispatchTests, TestOpLeftShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<<=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p <<= 2) <<= 1;
  ASSERT_EQ(v, 96);
}

TEST(ProxyDispatchTests, TestOpRightShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">>=">, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p >>= 2) >>= 1;
  ASSERT_EQ(v, 1);
}

TEST(ProxyDispatchTests, TestOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<",">, int(int val)>::build {};
  details::CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((*p, 6), 9);
}

TEST(ProxyDispatchTests, TestOpPtrToMem) {
  struct Base1 { int a; int b; int c; };
  struct Base2 { double x; };
  struct Derived1 : Base1 { int x; };
  struct Derived2 : Base2, Base1 { int d; };
  struct TestFacade : pro::facade_builder::add_direct_convention<pro::operator_dispatch<"->*">, int&(int Base1::* ptm)>::build {};
  Derived1 v1{};
  Derived2 v2{};
  pro::proxy<TestFacade> p1 = &v1, p2 = &v2;
  std::vector<int Base1::*> fields{&Base1::a, &Base1::b, &Base1::c};
  for (int i = 0; i < std::ssize(fields); ++i) {
    p1->*fields[i] = i + 1;
    p2->*fields[i] = i + 1;
  }
  ASSERT_EQ(v1.a, 1);
  ASSERT_EQ(v1.b, 2);
  ASSERT_EQ(v1.c, 3);
  ASSERT_EQ(v2.a, 1);
  ASSERT_EQ(v2.b, 2);
  ASSERT_EQ(v2.c, 3);
}

TEST(ProxyDispatchTests, TestOpParentheses) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"()">, int(int a, int b)>::build {};
  auto v = [](auto&&... args) { return (args + ...); };
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((*p)(2, 3), 5);
}

TEST(ProxyDispatchTests, TestOpBrackets) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"[]">, int&(int idx)>::build {};
  std::unordered_map<int, int> v;
  pro::proxy<TestFacade> p = &v;
  (*p)[3] = 12;
  ASSERT_EQ(v.size(), 1u);
  ASSERT_EQ(v.at(3), 12);
}

TEST(ProxyDispatchTests, TestRhsOpPlus) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"+", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 + *p, 14);
}

TEST(ProxyDispatchTests, TestRhsOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"-", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 - *p, -10);
}

TEST(ProxyDispatchTests, TestRhsOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"*", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 * *p, 24);
}

TEST(ProxyDispatchTests, TestRhsOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"/", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(50 / *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"%", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(26 % *p, 2);
}

TEST(ProxyDispatchTests, TestRhsOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"==", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 == *p, false);
  ASSERT_EQ(12 == *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"!=", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 != *p, true);
  ASSERT_EQ(12 != *p, false);
}

TEST(ProxyDispatchTests, TestRhsOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 > *p, false);
  ASSERT_EQ(13 > *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 < *p, false);
  ASSERT_EQ(11 < *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">=", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(11 >= *p, false);
  ASSERT_EQ(12 >= *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<=", true>, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(13 <= *p, false);
  ASSERT_EQ(12 <= *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<=>", true>, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 <=> *p, std::strong_ordering::less);
  ASSERT_EQ(12 <=> *p, std::strong_ordering::equal);
  ASSERT_EQ(20 <=> *p, std::strong_ordering::greater);
}

TEST(ProxyDispatchTests, TestRhsOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&&", true>, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(true && *p, true);
  ASSERT_EQ(false && *p, false);
}

TEST(ProxyDispatchTests, TestRhsOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"||", true>, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(false || *p, false);
  ASSERT_EQ(true || *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 & *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"|", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 | *p, 14);
}

TEST(ProxyDispatchTests, TestRhsOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"^", true>, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(8 ^ *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<<", true>, int(int val), std::ostream&(std::ostream& out)>::build {};
  int v = 2;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 << *p, 48);
  std::ostringstream stream;
  stream << *p;
  ASSERT_EQ(stream.str(), "2");
}

TEST(ProxyDispatchTests, TestRhsOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">>", true>, int(int val), std::istream&(std::istream& in)>::build {};
  int v = 1;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(25 >> *p, 12);
  std::istringstream stream("123");
  stream >> *p;
  ASSERT_EQ(v, 123);
}

TEST(ProxyDispatchTests, TestRhsOpPlusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"+=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs += *p), &lhs);
  ASSERT_EQ(lhs, 8);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpMinusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"-=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs -= *p), &lhs);
  ASSERT_EQ(lhs, 2);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpMultiplicationAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"*=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs *= *p), &lhs);
  ASSERT_EQ(lhs, 15);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpDivisionAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"/=", true>, void(int& val)>::build {};
  int lhs = 100, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs /= *p), &lhs);
  ASSERT_EQ(lhs, 33);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseAndAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"&=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs &= *p), &lhs);
  ASSERT_EQ(lhs, 1);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseOrAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"|=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs |= *p), &lhs);
  ASSERT_EQ(lhs, 7);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseXorAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"^=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs ^= *p), &lhs);
  ASSERT_EQ(lhs, 6);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpLeftShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"<<=", true>, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs <<= *p), &lhs);
  ASSERT_EQ(lhs, 40);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpRightShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<">>=", true>, void(int& val)>::build {};
  int lhs = 100, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs >>= *p), &lhs);
  ASSERT_EQ(lhs, 12);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<",", true>, int(int val)>::build {};
  details::CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((7, *p), 21);
}

TEST(ProxyDispatchTests, TestRhsOpPtrToMem) {
  struct TestFacade : pro::facade_builder::add_convention<pro::operator_dispatch<"->*", true>, int(int val)>::build {};
  details::PtrToMemTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2->**p, 6);
}

TEST(ProxyDispatchTests, TestIndirectConversion) {
  struct TestFacade : pro::facade_builder::add_convention<pro::conversion_dispatch<int>, int()>::build {};
  double v = 12.3;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(static_cast<int>(*p), 12);
}

TEST(ProxyDispatchTests, TestDirectConversion) {
  struct TestFacadeBase : pro::facade_builder
      ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream& out)>
      ::build {};
  struct TestFacade : pro::facade_builder
      ::add_facade<TestFacadeBase>
      ::add_convention<pro::operator_dispatch<"+=">, void(int val)>
      ::add_direct_convention<pro::conversion_dispatch<pro::proxy<TestFacadeBase>>, pro::proxy<TestFacadeBase>() &&>
      ::build {};
  pro::proxy<TestFacade> p1 = std::make_unique<int>(123);
  *p1 += 3;
  pro::proxy<TestFacadeBase> p2 = static_cast<pro::proxy<TestFacadeBase>>(std::move(p1));
  ASSERT_FALSE(p1.has_value());
  std::ostringstream stream;
  stream << *p2;
  ASSERT_EQ(stream.str(), "126");
}

TEST(ProxyDispatchTests, TestImplciitConversion) {
  struct TestFacade : pro::facade_builder::add_convention<pro::conversion_dispatch<int, false>, int()>::build {};
  double v = 12.3;
  pro::proxy<TestFacade> p = &v;
  int converted = *p;
  ASSERT_EQ(converted, 12);
}

TEST(ProxyDispatchTests, TestFreeAsMemDispatch) {
  struct TestFacade : pro::facade_builder::add_convention<details::FreeMemToString, std::string() const>::build {};
  int v = 123;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p->ToString(), "123");
}
