// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <iomanip>
#include <memory>
#include <memory_resource>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>
#include "proxy.h"

namespace {

struct Draw : pro::dispatch<void(std::ostream&)> {
  template <class T>
  void operator()(const T& self, std::ostream& out) { self.Draw(out); }
};
struct Area : pro::dispatch<double()> {
  template <class T>
  double operator()(const T& self) { return self.Area(); }
};

struct DrawableFacade : pro::facade<Draw, Area> {};

class Rectangle {
 public:
  void Draw(std::ostream& out) const
      { out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}"; }
  void SetWidth(double width) { width_ = width; }
  void SetHeight(double height) { height_ = height; }
  double Area() const { return width_ * height_; }

 private:
  double width_;
  double height_;
};

class Circle {
 public:
  void Draw(std::ostream& out) const { out << "{Circle: radius = " << radius_ << "}"; }
  void SetRadius(double radius) { radius_ = radius; }
  double Area() const { return std::numbers::pi * radius_ * radius_; }

 private:
  double radius_;
};

class Point {
 public:
  void Draw(std::ostream& out) const { out << "{Point}"; }
  constexpr double Area() const { return 0; }
};

std::string PrintDrawableToString(pro::proxy<DrawableFacade> p) {
  std::stringstream result;
  result << std::fixed << std::setprecision(5) << "shape = ";
  p.invoke<Draw>(result);
  result << ", area = " << p.invoke<Area>();
  return std::move(result).str();
}

std::vector<std::string> ParseCommand(const std::string& s) {
  std::vector<std::string> result(1u);
  bool in_quote = false;
  std::size_t last_valid = s.find_last_not_of(' ');
  for (std::size_t i = 0u; i <= last_valid; ++i) {
    if (s[i] == '`' && i < last_valid) {
      result.back() += s[++i];
    } else if (s[i] == '"') {
      in_quote = !in_quote;
    } else if (s[i] == ' ' && !in_quote) {
      if (!result.back().empty()) {
        result.emplace_back();
      }
    } else {
      result.back() += s[i];
    }
  }
  if (result.back().empty()) {
    result.pop_back();
  }
  return result;
}

pro::proxy<DrawableFacade> MakeDrawableFromCommand(const std::string& s) {
  std::vector<std::string> parsed = ParseCommand(s);
  if (!parsed.empty()) {
    if (parsed[0u] == "Rectangle") {
      if (parsed.size() == 3u) {
        static std::pmr::unsynchronized_pool_resource rectangle_memory_pool;
        std::pmr::polymorphic_allocator<> alloc{&rectangle_memory_pool};
        auto deleter = [alloc](Rectangle* ptr) mutable
            { alloc.delete_object<Rectangle>(ptr); };
        Rectangle* instance = alloc.new_object<Rectangle>();
        std::unique_ptr<Rectangle, decltype(deleter)> p{instance, deleter};
        p->SetWidth(std::stod(parsed[1u]));
        p->SetHeight(std::stod(parsed[2u]));
        return p;
      }
    } else if (parsed[0u] == "Circle") {
      if (parsed.size() == 2u) {
        Circle circle;
        circle.SetRadius(std::stod(parsed[1u]));
        return pro::make_proxy<DrawableFacade>(circle);
      }
    } else if (parsed[0u] == "Point") {
      if (parsed.size() == 1u) {
        static Point instance;
        return &instance;
      }
    }
  }
  throw std::runtime_error{"Invalid command"};
}

}  // namespace

TEST(ProxyIntegrationTests, TestDrawable) {
  pro::proxy<DrawableFacade> p = MakeDrawableFromCommand("Rectangle 2 3");
  std::string s = PrintDrawableToString(std::move(p));
  ASSERT_EQ(s, "shape = {Rectangle: width = 2.00000, height = 3.00000}, area = 6.00000");

  p = MakeDrawableFromCommand("Circle 1");
  s = PrintDrawableToString(std::move(p));
  ASSERT_EQ(s, "shape = {Circle: radius = 1.00000}, area = 3.14159");

  p = MakeDrawableFromCommand("Point");
  s = PrintDrawableToString(std::move(p));
  ASSERT_EQ(s, "shape = {Point}, area = 0.00000");

  try {
    p = MakeDrawableFromCommand("Triangle 2 3");
  } catch (const std::runtime_error& e) {
    ASSERT_STREQ(e.what(), "Invalid command");
  }
}
