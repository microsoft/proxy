// Copyright (c) Microsoft Corporation. All rights reserved.
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

namespace {

PRO_DEF_OPERATOR_DISPATCH(OpPlus, "+");
PRO_DEF_OPERATOR_DISPATCH(OpMinus, "-");
PRO_DEF_OPERATOR_DISPATCH(OpAsterisk, "*");
PRO_DEF_OPERATOR_DISPATCH(OpSlash, "/");
PRO_DEF_OPERATOR_DISPATCH(OpPercent, "%");
PRO_DEF_OPERATOR_DISPATCH(OpIncrement, "++");
PRO_DEF_OPERATOR_DISPATCH(OpDecrement, "--");
PRO_DEF_OPERATOR_DISPATCH(OpEqualTo, "==");
PRO_DEF_OPERATOR_DISPATCH(OpNotEqualTo, "!=");
PRO_DEF_OPERATOR_DISPATCH(OpGreaterThan, ">");
PRO_DEF_OPERATOR_DISPATCH(OpLessThan, "<");
PRO_DEF_OPERATOR_DISPATCH(OpGreaterThanOrEqualTo, ">=");
PRO_DEF_OPERATOR_DISPATCH(OpLessThanOrEqualTo, "<=");
PRO_DEF_OPERATOR_DISPATCH(OpSpaceship, "<=>");
PRO_DEF_OPERATOR_DISPATCH(OpLogicalNot, "!");
PRO_DEF_OPERATOR_DISPATCH(OpLogicalAnd, "&&");
PRO_DEF_OPERATOR_DISPATCH(OpLogicalOr, "||");
PRO_DEF_OPERATOR_DISPATCH(OpTilde, "~");
PRO_DEF_OPERATOR_DISPATCH(OpAmpersand, "&");
PRO_DEF_OPERATOR_DISPATCH(OpPipe, "|");
PRO_DEF_OPERATOR_DISPATCH(OpCaret, "^");
PRO_DEF_OPERATOR_DISPATCH(OpLeftShift, "<<");
PRO_DEF_OPERATOR_DISPATCH(OpRightShift, ">>");
PRO_DEF_DIRECT_OPERATOR_DISPATCH(DirectOpPlusAssignment, "+=");
PRO_DEF_OPERATOR_DISPATCH(OpPlusAssignment, "+=");
PRO_DEF_OPERATOR_DISPATCH(OpMinusAssignment, "-=");
PRO_DEF_OPERATOR_DISPATCH(OpMultiplicationAssignment, "*=");
PRO_DEF_OPERATOR_DISPATCH(OpDivisionAssignment, "/=");
PRO_DEF_OPERATOR_DISPATCH(OpBitwiseAndAssignment, "&=");
PRO_DEF_OPERATOR_DISPATCH(OpBitwiseOrAssignment, "|=");
PRO_DEF_OPERATOR_DISPATCH(OpBitwiseXorAssignment, "^=");
PRO_DEF_OPERATOR_DISPATCH(OpLeftShiftAssignment, "<<=");
PRO_DEF_OPERATOR_DISPATCH(OpRightShiftAssignment, ">>=");
PRO_DEF_OPERATOR_DISPATCH(OpComma, ",");
PRO_DEF_DIRECT_OPERATOR_DISPATCH(OpPtrToMem, "->*");
PRO_DEF_OPERATOR_DISPATCH(OpParentheses, "()");
PRO_DEF_OPERATOR_DISPATCH(OpBrackets, "[]");

PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpPlus, "+");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpMinus, "-");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpAsterisk, "*");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpSlash, "/");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpPercent, "%");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpEqualTo, "==");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpNotEqualTo, "!=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpGreaterThan, ">");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLessThan, "<");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpGreaterThanOrEqualTo, ">=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLessThanOrEqualTo, "<=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpSpaceship, "<=>");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLogicalAnd, "&&");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLogicalOr, "||");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpAmpersand, "&");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpPipe, "|");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpCaret, "^");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLeftShift, "<<");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpRightShift, ">>");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpPlusAssignment, "+=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpMinusAssignment, "-=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpMultiplicationAssignment, "*=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpDivisionAssignment, "/=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpBitwiseAndAssignment, "&=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpBitwiseOrAssignment, "|=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpBitwiseXorAssignment, "^=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpLeftShiftAssignment, "<<=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpRightShiftAssignment, ">>=");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpComma, ",");
PRO_DEF_RHS_OPERATOR_DISPATCH(RhsOpPtrToMem, "->*");

PRO_DEF_CONVERSION_DISPATCH(ConvertToInt, int);
template <class F>
PRO_DEF_DIRECT_CONVERSION_DISPATCH(ConvertToBase, pro::proxy<F>);

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
}  // namespace

TEST(ProxyDispatchTests, TestOpPlus) {
  struct TestFacade : pro::facade_builder::add_convention<OpPlus, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(+*p, 12);
  ASSERT_EQ(*p + 2, 14);
}

TEST(ProxyDispatchTests, TestOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<OpMinus, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(-*p, -12);
  ASSERT_EQ(*p - 2, 10);
}

TEST(ProxyDispatchTests, TestOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<OpAsterisk, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p * 2, 24);
}

TEST(ProxyDispatchTests, TestOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<OpSlash, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p / 2, 6);
}

TEST(ProxyDispatchTests, TestOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<OpPercent, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p % 5, 2);
}

TEST(ProxyDispatchTests, TestOpIncrement) {
  struct TestFacade : pro::facade_builder::add_convention<OpIncrement, int(), int(int)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(++(*p), 13);
  ASSERT_EQ((*p)++, 13);
  ASSERT_EQ(v, 14);
}

TEST(ProxyDispatchTests, TestOpDecrement) {
  struct TestFacade : pro::facade_builder::add_convention<OpDecrement, int(), int(int)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(--(*p), 11);
  ASSERT_EQ((*p)--, 11);
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p == 12, true);
  ASSERT_EQ(*p == 11, false);
}

TEST(ProxyDispatchTests, TestOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpNotEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p != 12, false);
  ASSERT_EQ(*p != 11, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<OpGreaterThan, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p > 2, true);
  ASSERT_EQ(*p > 20, false);
}

TEST(ProxyDispatchTests, TestOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<OpLessThan, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p < 2, false);
  ASSERT_EQ(*p < 20, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpGreaterThanOrEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p >= 20, false);
  ASSERT_EQ(*p >= 12, true);
}

TEST(ProxyDispatchTests, TestOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpLessThanOrEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p <= 2, false);
  ASSERT_EQ(*p <= 12, true);
}

TEST(ProxyDispatchTests, TestOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<OpSpaceship, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p <=> 2, std::strong_ordering::greater);
  ASSERT_EQ(*p <=> 12, std::strong_ordering::equal);
  ASSERT_EQ(*p <=> 20, std::strong_ordering::less);
}

TEST(ProxyDispatchTests, TestOpLogicalNot) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalNot, bool()>::build {};
  int v1 = 12, v2 = 0;
  pro::proxy<TestFacade> p1 = &v1, p2 = &v2;
  ASSERT_EQ(!*p1, false);
  ASSERT_EQ(!*p2, true);
}

TEST(ProxyDispatchTests, TestOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalAnd, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p && true, true);
  ASSERT_EQ(*p && false, false);
}

TEST(ProxyDispatchTests, TestOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalOr, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p || true, true);
  ASSERT_EQ(*p || false, false);
}

TEST(ProxyDispatchTests, TestOpTilde) {
  struct TestFacade : pro::facade_builder::add_convention<OpTilde, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(~*p, -13);
}

TEST(ProxyDispatchTests, TestOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<OpAmpersand, const void* () noexcept, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(&*p, &v);
  ASSERT_EQ(*p & 4, 4);
}

TEST(ProxyDispatchTests, TestOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<OpPipe, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p | 6, 14);
}

TEST(ProxyDispatchTests, TestOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<OpCaret, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p ^ 5, 9);
}

TEST(ProxyDispatchTests, TestOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<OpLeftShift, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p << 2, 48);
}

TEST(ProxyDispatchTests, TestOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<OpRightShift, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p >> 2, 3);
}

TEST(ProxyDispatchTests, TestOpPlusAssignment) {
  struct TestFacade : pro::facade_builder
      ::add_convention<OpPlusAssignment, void(int val)>
      ::add_convention<DirectOpPlusAssignment, void(int val)>
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
  struct TestFacade : pro::facade_builder::add_convention<OpMinusAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p -= 2) -= 3;
  ASSERT_EQ(v, 7);
}

TEST(ProxyDispatchTests, TestOpMultiplicationAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpMultiplicationAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p *= 2) *= 3;
  ASSERT_EQ(v, 72);
}

TEST(ProxyDispatchTests, TestOpDivisionAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpDivisionAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p /= 2) /= 2;
  ASSERT_EQ(v, 3);
}

TEST(ProxyDispatchTests, TestOpBitwiseAndAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseAndAssignment, void(int val)>::build {};
  int v = 15;
  pro::proxy<TestFacade> p = &v;
  (*p &= 11) &= 14;
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpBitwiseOrAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseOrAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p |= 2) |= 1;
  ASSERT_EQ(v, 15);
}

TEST(ProxyDispatchTests, TestOpBitwiseXorAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseXorAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p ^= 6) ^= 1;
  ASSERT_EQ(v, 11);
}

TEST(ProxyDispatchTests, TestOpLeftShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpLeftShiftAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p <<= 2) <<= 1;
  ASSERT_EQ(v, 96);
}

TEST(ProxyDispatchTests, TestOpRightShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpRightShiftAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  (*p >>= 2) >>= 1;
  ASSERT_EQ(v, 1);
}

TEST(ProxyDispatchTests, TestOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<OpComma, int(int val)>::build {};
  CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((*p, 6), 9);
}

TEST(ProxyDispatchTests, TestOpPtrToMem) {
  struct Base1 { int a; int b; int c; };
  struct Base2 { double x; };
  struct Derived1 : Base1 { int x; };
  struct Derived2 : Base2, Base1 { int d; };
  struct TestFacade : pro::facade_builder::add_convention<OpPtrToMem, int&(int Base1::* ptm)>::build {};
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
  struct TestFacade : pro::facade_builder::add_convention<OpParentheses, int(int a, int b)>::build {};
  auto v = [](auto&&... args) { return (args + ...); };
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((*p)(2, 3), 5);
}

TEST(ProxyDispatchTests, TestOpBrackets) {
  struct TestFacade : pro::facade_builder::add_convention<OpBrackets, int&(int idx)>::build {};
  std::unordered_map<int, int> v;
  pro::proxy<TestFacade> p = &v;
  (*p)[3] = 12;
  ASSERT_EQ(v.size(), 1u);
  ASSERT_EQ(v.at(3), 12);
}

TEST(ProxyDispatchTests, TestRhsOpPlus) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpPlus, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 + *p, 14);
}

TEST(ProxyDispatchTests, TestRhsOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpMinus, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 - *p, -10);
}

TEST(ProxyDispatchTests, TestRhsOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpAsterisk, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 * *p, 24);
}

TEST(ProxyDispatchTests, TestRhsOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpSlash, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(50 / *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpPercent, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(26 % *p, 2);
}

TEST(ProxyDispatchTests, TestRhsOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 == *p, false);
  ASSERT_EQ(12 == *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpNotEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 != *p, true);
  ASSERT_EQ(12 != *p, false);
}

TEST(ProxyDispatchTests, TestRhsOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpGreaterThan, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 > *p, false);
  ASSERT_EQ(13 > *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLessThan, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 < *p, false);
  ASSERT_EQ(11 < *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpGreaterThanOrEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(11 >= *p, false);
  ASSERT_EQ(12 >= *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLessThanOrEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(13 <= *p, false);
  ASSERT_EQ(12 <= *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpSpaceship, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 <=> *p, std::strong_ordering::less);
  ASSERT_EQ(12 <=> *p, std::strong_ordering::equal);
  ASSERT_EQ(20 <=> *p, std::strong_ordering::greater);
}

TEST(ProxyDispatchTests, TestRhsOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLogicalAnd, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(true && *p, true);
  ASSERT_EQ(false && *p, false);
}

TEST(ProxyDispatchTests, TestRhsOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLogicalOr, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(false || *p, false);
  ASSERT_EQ(true || *p, true);
}

TEST(ProxyDispatchTests, TestRhsOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpAmpersand, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 & *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpPipe, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 | *p, 14);
}

TEST(ProxyDispatchTests, TestRhsOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpCaret, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(8 ^ *p, 4);
}

TEST(ProxyDispatchTests, TestRhsOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLeftShift, int(int val), std::ostream&(std::ostream& out)>::build {};
  int v = 2;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 << *p, 48);
  std::ostringstream stream;
  stream << *p;
  ASSERT_EQ(stream.str(), "2");
}

TEST(ProxyDispatchTests, TestRhsOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpRightShift, int(int val), std::istream&(std::istream& in)>::build {};
  int v = 1;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(25 >> *p, 12);
  std::istringstream stream("123");
  stream >> *p;
  ASSERT_EQ(v, 123);
}

TEST(ProxyDispatchTests, TestRhsOpPlusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpPlusAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs += *p), &lhs);
  ASSERT_EQ(lhs, 8);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpMinusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpMinusAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs -= *p), &lhs);
  ASSERT_EQ(lhs, 2);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpMultiplicationAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpMultiplicationAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs *= *p), &lhs);
  ASSERT_EQ(lhs, 15);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpDivisionAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpDivisionAssignment, void(int& val)>::build {};
  int lhs = 100, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs /= *p), &lhs);
  ASSERT_EQ(lhs, 33);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseAndAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpBitwiseAndAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs &= *p), &lhs);
  ASSERT_EQ(lhs, 1);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseOrAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpBitwiseOrAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs |= *p), &lhs);
  ASSERT_EQ(lhs, 7);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpBitwiseXorAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpBitwiseXorAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs ^= *p), &lhs);
  ASSERT_EQ(lhs, 6);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpLeftShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpLeftShiftAssignment, void(int& val)>::build {};
  int lhs = 5, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs <<= *p), &lhs);
  ASSERT_EQ(lhs, 40);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpRightShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpRightShiftAssignment, void(int& val)>::build {};
  int lhs = 100, rhs = 3;
  pro::proxy<TestFacade> p = &rhs;
  ASSERT_EQ(&(lhs >>= *p), &lhs);
  ASSERT_EQ(lhs, 12);
  ASSERT_EQ(rhs, 3);
}

TEST(ProxyDispatchTests, TestRhsOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpComma, int(int val)>::build {};
  CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((7, *p), 21);
}

TEST(ProxyDispatchTests, TestRhsOpPtrToMem) {
  struct TestFacade : pro::facade_builder::add_convention<RhsOpPtrToMem, int(int val)>::build {};
  PtrToMemTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2->**p, 6);
}

TEST(ProxyDispatchTests, TestIndirectConversion) {
  struct TestFacade : pro::facade_builder::add_convention<ConvertToInt, int()>::build {};
  double v = 12.3;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(static_cast<int>(*p), 12);
}

TEST(ProxyDispatchTests, TestDirectConversion) {
  struct TestFacadeBase : pro::facade_builder
      ::add_convention<RhsOpLeftShift, std::ostream&(std::ostream& out)>
      ::build {};
  struct TestFacade : pro::facade_builder
      ::add_facade<TestFacadeBase>
      ::add_convention<OpPlusAssignment, void(int val)>
      ::add_convention<ConvertToBase<TestFacadeBase>, pro::proxy<TestFacadeBase>() &&>
      ::build {};
  pro::proxy<TestFacade> p1 = std::make_unique<int>(123);
  *p1 += 3;
  pro::proxy<TestFacadeBase> p2 = static_cast<pro::proxy<TestFacadeBase>>(std::move(p1));
  std::ostringstream stream;
  stream << *p2;
  ASSERT_EQ(stream.str(), "126");
}
