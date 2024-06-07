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
PRO_DEF_OPERATOR_DISPATCH(OpPtrToMem, "->*");
PRO_DEF_OPERATOR_DISPATCH(OpArrow, "->");
PRO_DEF_OPERATOR_DISPATCH(OpParentheses, "()");
PRO_DEF_OPERATOR_DISPATCH(OpBrackets, "[]");

PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpPlus, "+");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpMinus, "-");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpAsterisk, "*");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpSlash, "/");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpPercent, "%");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpIncrement, "++");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpDecrement, "--");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpEqualTo, "==");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpNotEqualTo, "!=");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpGreaterThan, ">");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpLessThan, "<");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpGreaterThanOrEqualTo, ">=");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpLessThanOrEqualTo, "<=");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpSpaceship, "<=>");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpLogicalAnd, "&&");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpLogicalOr, "||");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpAmpersand, "&");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpPipe, "|");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpCaret, "^");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpLeftShift, "<<");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpRightShift, ">>");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpComma, ",");
PRO_DEF_PREFIX_OPERATOR_DISPATCH(PreOpPtrToMem, "->*");

PRO_DEF_CONVERSION_DISPATCH(ConvertToInt, int);

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
  struct TestFacade : pro::facade_builder::add_convention<OpPlus, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p + 2, 14);
}

TEST(ProxyDispatchTests, TestOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<OpMinus, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p - 2, 10);
}

TEST(ProxyDispatchTests, TestOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<OpAsterisk, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p * 2, 24);
}

TEST(ProxyDispatchTests, TestOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<OpSlash, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p / 2, 6);
}

TEST(ProxyDispatchTests, TestOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<OpPercent, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p % 5, 2);
}

TEST(ProxyDispatchTests, TestOpIncrement) {
  struct TestFacade : pro::facade_builder::add_convention<OpIncrement, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p++, 12);
  ASSERT_EQ(v, 13);
}

TEST(ProxyDispatchTests, TestOpDecrement) {
  struct TestFacade : pro::facade_builder::add_convention<OpDecrement, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p--, 12);
  ASSERT_EQ(v, 11);
}

TEST(ProxyDispatchTests, TestOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p == 12, true);
  ASSERT_EQ(p == 11, false);
}

TEST(ProxyDispatchTests, TestOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpNotEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p != 12, false);
  ASSERT_EQ(p != 11, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<OpGreaterThan, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p > 2, true);
  ASSERT_EQ(p > 20, false);
}

TEST(ProxyDispatchTests, TestOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<OpLessThan, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p < 2, false);
  ASSERT_EQ(p < 20, true);
}

TEST(ProxyDispatchTests, TestOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpGreaterThanOrEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p >= 20, false);
  ASSERT_EQ(p >= 12, true);
}

TEST(ProxyDispatchTests, TestOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<OpLessThanOrEqualTo, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p <= 2, false);
  ASSERT_EQ(p <= 12, true);
}

TEST(ProxyDispatchTests, TestOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<OpSpaceship, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p <=> 2, std::strong_ordering::greater);
  ASSERT_EQ(p <=> 12, std::strong_ordering::equal);
  ASSERT_EQ(p <=> 20, std::strong_ordering::less);
}

TEST(ProxyDispatchTests, TestOpLogicalNot) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalNot, bool()>::build {};
  int v1 = 12, v2 = 0;
  pro::proxy<TestFacade> p1 = &v1, p2 = &v2;
  ASSERT_EQ(!p1, false);
  ASSERT_EQ(!p2, true);
}

TEST(ProxyDispatchTests, TestOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalAnd, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p && true, true);
  ASSERT_EQ(p && false, false);
}

TEST(ProxyDispatchTests, TestOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<OpLogicalOr, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(v || true, true);
  ASSERT_EQ(v || false, false);
}

TEST(ProxyDispatchTests, TestOpTilde) {
  struct TestFacade : pro::facade_builder::add_convention<OpTilde, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(~v, -13);
}

TEST(ProxyDispatchTests, TestOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<OpAmpersand, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p & 4, 4);
}

TEST(ProxyDispatchTests, TestOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<OpPipe, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p | 6, 14);
}

TEST(ProxyDispatchTests, TestOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<OpCaret, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p ^ 5, 9);
}

TEST(ProxyDispatchTests, TestOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<OpLeftShift, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p << 2, 48);
}

TEST(ProxyDispatchTests, TestOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<OpRightShift, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p >> 2, 3);
}

TEST(ProxyDispatchTests, TestOpPlusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpPlusAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p += 2).has_value());
  ASSERT_EQ(v, 14);
}

TEST(ProxyDispatchTests, TestOpMinusAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpMinusAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p -= 2).has_value());
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpMultiplicationAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpMultiplicationAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p *= 2).has_value());
  ASSERT_EQ(v, 24);
}

TEST(ProxyDispatchTests, TestOpDivisionAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpDivisionAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p /= 2).has_value());
  ASSERT_EQ(v, 6);
}

TEST(ProxyDispatchTests, TestOpBitwiseAndAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseAndAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p &= 6).has_value());
  ASSERT_EQ(v, 4);
}

TEST(ProxyDispatchTests, TestOpBitwiseOrAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseOrAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p |= 2).has_value());
  ASSERT_EQ(v, 14);
}

TEST(ProxyDispatchTests, TestOpBitwiseXorAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpBitwiseXorAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p ^= 6).has_value());
  ASSERT_EQ(v, 10);
}

TEST(ProxyDispatchTests, TestOpLeftShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpLeftShiftAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p <<= 2).has_value());
  ASSERT_EQ(v, 48);
}

TEST(ProxyDispatchTests, TestOpRightShiftAssignment) {
  struct TestFacade : pro::facade_builder::add_convention<OpRightShiftAssignment, void(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_TRUE((p >>= 2).has_value());
  ASSERT_EQ(v, 3);
}

TEST(ProxyDispatchTests, TestOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<OpComma, int(int val)>::build {};
  CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((p, 6), 9);
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

TEST(ProxyDispatchTests, TestOpArrow) {
  struct TestFacade : pro::facade_builder::add_convention<OpArrow, const void*()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(std::to_address(p), &v);
}

TEST(ProxyDispatchTests, TestOpParentheses) {
  struct TestFacade : pro::facade_builder::add_convention<OpParentheses, int(int a, int b)>::build {};
  auto v = [](auto&&... args) { return (args + ...); };
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(p(2, 3), 5);
}

TEST(ProxyDispatchTests, TestOpBrackets) {
  struct TestFacade : pro::facade_builder::add_convention<OpBrackets, int&(int idx)>::build {};
  std::unordered_map<int, int> v;
  pro::proxy<TestFacade> p = &v;
  p[3] = 12;
  ASSERT_EQ(v.size(), 1u);
  ASSERT_EQ(v.at(3), 12);
}

TEST(ProxyDispatchTests, TestPreOpPlus) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpPlus, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(+p, 12);
  ASSERT_EQ(2 + p, 14);
}

TEST(ProxyDispatchTests, TestPreOpMinus) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpMinus, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(-p, -12);
  ASSERT_EQ(2 - p, -10);
}

TEST(ProxyDispatchTests, TestPreOpAsterisk) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpAsterisk, int(), int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(*p, 12);
  ASSERT_EQ(2 * p, 24);
}

TEST(ProxyDispatchTests, TestPreOpSlash) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpSlash, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(50 / p, 4);
}

TEST(ProxyDispatchTests, TestPreOpPercent) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpPercent, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(26 % p, 2);
}

TEST(ProxyDispatchTests, TestPreOpIncrement) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpIncrement, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(++p, 13);
  ASSERT_EQ(v, 13);
}

TEST(ProxyDispatchTests, TestPreOpDecrement) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpDecrement, int()>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(--p, 11);
  ASSERT_EQ(v, 11);
}

TEST(ProxyDispatchTests, TestPreOpEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 == p, false);
  ASSERT_EQ(12 == p, true);
}

TEST(ProxyDispatchTests, TestPreOpNotEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpNotEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 != p, true);
  ASSERT_EQ(12 != p, false);
}

TEST(ProxyDispatchTests, TestPreOpGreaterThan) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpGreaterThan, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 > p, false);
  ASSERT_EQ(13 > p, true);
}

TEST(ProxyDispatchTests, TestPreOpLessThan) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpLessThan, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 < p, false);
  ASSERT_EQ(11 < p, true);
}

TEST(ProxyDispatchTests, TestPreOpGreaterThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpGreaterThanOrEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(11 >= p, false);
  ASSERT_EQ(12 >= p, true);
}

TEST(ProxyDispatchTests, TestPreOpLessThanOrEqualTo) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpLessThanOrEqualTo, bool(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(13 <= p, false);
  ASSERT_EQ(12 <= p, true);
}

TEST(ProxyDispatchTests, TestPreOpSpaceship) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpSpaceship, std::strong_ordering(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2 <=> p, std::strong_ordering::less);
  ASSERT_EQ(12 <=> p, std::strong_ordering::equal);
  ASSERT_EQ(20 <=> p, std::strong_ordering::greater);
}

TEST(ProxyDispatchTests, TestPreOpLogicalAnd) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpLogicalAnd, bool(bool val)>::build {};
  bool v = true;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(true && p, true);
  ASSERT_EQ(false && p, false);
}

TEST(ProxyDispatchTests, TestPreOpLogicalOr) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpLogicalOr, bool(bool val)>::build {};
  bool v = false;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(false || p, false);
  ASSERT_EQ(true || p, true);
}

TEST(ProxyDispatchTests, TestPreOpAmpersand) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpAmpersand, const void*() noexcept, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 & p, 4);
  ASSERT_EQ(&p, &v);
}

TEST(ProxyDispatchTests, TestPreOpPipe) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpPipe, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(6 | p, 14);
}

TEST(ProxyDispatchTests, TestPreOpCaret) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpCaret, int(int val)>::build {};
  int v = 12;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(8 ^ p, 4);
}

TEST(ProxyDispatchTests, TestPreOpLeftShift) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpLeftShift, int(int val), std::ostream&(std::ostream& out)>::build {};
  int v = 2;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(12 << p, 48);
  std::ostringstream stream;
  stream << p;
  ASSERT_EQ(stream.str(), "2");
}

TEST(ProxyDispatchTests, TestPreOpRightShift) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpRightShift, int(int val), std::istream&(std::istream& in)>::build {};
  int v = 1;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(25 >> p, 12);
  std::istringstream stream("123");
  stream >> p;
  ASSERT_EQ(v, 123);
}

TEST(ProxyDispatchTests, TestPreOpComma) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpComma, int(int val)>::build {};
  CommaTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ((7, p), 21);
}

TEST(ProxyDispatchTests, TestPreOpPtrToMem) {
  struct TestFacade : pro::facade_builder::add_convention<PreOpPtrToMem, int(int val)>::build {};
  PtrToMemTester v{3};
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(2->*p, 6);
}

TEST(ProxyDispatchTests, TestConversion) {
  struct TestFacade : pro::facade_builder::add_convention<ConvertToInt, int()>::build {};
  double v = 12.3;
  pro::proxy<TestFacade> p = &v;
  ASSERT_EQ(static_cast<int>(p), 12);
}
