#include <iostream>

import proxy;
import foo;
import foo_impl;

auto user(pro::proxy<Foo> p) {
  std::cout << "Foo says: " << p->GetFoo() << '\n';
}

int main() {
  MyFoo foo;
  user(&foo);
}