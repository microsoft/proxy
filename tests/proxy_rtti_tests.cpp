// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <proxy/proxy.h>
#include <vector>

namespace proxy_rtti_tests_details {

struct TestFacade : pro::facade_builder                   //
                    ::add_skill<pro::skills::rtti>        //
                    ::add_skill<pro::skills::direct_rtti> //
                    ::build {};

} // namespace proxy_rtti_tests_details

namespace details = proxy_rtti_tests_details;

TEST(ProxyRttiTests, TestIndirectCast_Ref_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  proxy_cast<int&>(*p) = 456;
  ASSERT_EQ(v, 456);
}

TEST(ProxyRttiTests, TestIndirectCast_Ref_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<double&>(*p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestIndirectCast_ConstRef_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  const int& r = proxy_cast<const int&>(*p);
  ASSERT_EQ(&v, &r);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestIndirectCast_ConstRef_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<const double&>(*p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestIndirectCast_Copy_Succeed) {
  int v1 = 123;
  pro::proxy<details::TestFacade> p = &v1;
  int v2 = proxy_cast<int>(*p);
  ASSERT_EQ(v1, 123);
  ASSERT_EQ(v2, 123);
}

TEST(ProxyRttiTests, TestIndirectCast_Copy_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<double>(*p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestIndirectCast_Move_Succeed) {
  const std::vector<int> v1{1, 2, 3};
  auto p = pro::make_proxy<details::TestFacade>(v1);
  auto v2 = proxy_cast<std::vector<int>>(std::move(*p));
  ASSERT_EQ(v2, v1);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyRttiTests, TestIndirectCast_Move_Fail) {
  auto p = pro::make_proxy<details::TestFacade>(std::vector<int>{1, 2, 3});
  bool exception_thrown = false;
  try {
    proxy_cast<std::vector<double>>(std::move(*p));
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestIndirectCast_Ptr_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<int>(&*p);
  static_assert(std::is_same_v<decltype(ptr), int*>);
  ASSERT_EQ(ptr, &v);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestIndirectCast_Ptr_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<double>(&*p);
  static_assert(std::is_same_v<decltype(ptr), double*>);
  ASSERT_EQ(ptr, nullptr);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestIndirectCast_ConstPtr_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<const int>(&*p);
  static_assert(std::is_same_v<decltype(ptr), const int*>);
  ASSERT_EQ(ptr, &v);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestIndirectCast_ConstPtr_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<const double>(&*p);
  static_assert(std::is_same_v<decltype(ptr), const double*>);
  ASSERT_EQ(ptr, nullptr);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestIndirectTypeid) {
  int a = 123;
  pro::proxy<details::TestFacade> p = &a;
  ASSERT_EQ(proxy_typeid(*p), typeid(int));
}

TEST(ProxyRttiTests, TestDirectCast_Ref_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  *proxy_cast<int*&>(p) = 456;
  ASSERT_EQ(v, 456);
}

TEST(ProxyRttiTests, TestDirectCast_Ref_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<double*&>(p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestDirectCast_ConstRef_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  int* const& r = proxy_cast<int* const&>(p);
  ASSERT_EQ(&v, r);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestDirectCast_ConstRef_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<double* const&>(p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestDirectCast_Copy_Succeed) {
  int v1 = 123;
  pro::proxy<details::TestFacade> p = &v1;
  int* v2 = proxy_cast<int*>(p);
  ASSERT_EQ(v2, &v1);
  ASSERT_EQ(v1, 123);
}

TEST(ProxyRttiTests, TestDirectCast_Copy_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  bool exception_thrown = false;
  try {
    proxy_cast<double*>(p);
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
}

TEST(ProxyRttiTests, TestDirectCast_Move_Succeed) {
  int v1 = 123;
  pro::proxy<details::TestFacade> p = &v1;
  auto v2 = proxy_cast<int*>(std::move(p));
  ASSERT_EQ(v2, &v1);
  ASSERT_EQ(v1, 123);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyRttiTests, TestDirectCast_Move_Fail) {
  int v1 = 123;
  pro::proxy<details::TestFacade> p = &v1;
  bool exception_thrown = false;
  try {
    proxy_cast<double*>(std::move(p));
  } catch (const pro::bad_proxy_cast&) {
    exception_thrown = true;
  }
  ASSERT_TRUE(exception_thrown);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyRttiTests, TestDirectCast_Ptr_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<int*>(&p);
  static_assert(std::is_same_v<decltype(ptr), int**>);
  ASSERT_EQ(*ptr, &v);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestDirectCast_Ptr_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<double*>(&p);
  static_assert(std::is_same_v<decltype(ptr), double**>);
  ASSERT_EQ(ptr, nullptr);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestDirectCast_ConstPtr_Succeed) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<int* const>(&p);
  static_assert(std::is_same_v<decltype(ptr), int* const*>);
  ASSERT_EQ(*ptr, &v);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestDirectCast_ConstPtr_Fail) {
  int v = 123;
  pro::proxy<details::TestFacade> p = &v;
  auto ptr = proxy_cast<double* const>(&*p);
  static_assert(std::is_same_v<decltype(ptr), double* const*>);
  ASSERT_EQ(ptr, nullptr);
  ASSERT_EQ(v, 123);
}

TEST(ProxyRttiTests, TestDirectTypeid) {
  int a = 123;
  pro::proxy<details::TestFacade> p = &a;
  ASSERT_EQ(proxy_typeid(p), typeid(int*));
}
