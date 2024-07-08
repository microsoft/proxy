#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int)>
    ::build {};

// This is a function, rather than a function template
void PrintDictionary(pro::proxy<Dictionary> dictionary) {
  std::cout << dictionary->at(1) << "\n";
}

int main() {
  static std::map<int, std::string> container1{{1, "hello"}};
  auto container2 = std::make_shared<std::vector<const char*>>();
  container2->push_back("hello");
  container2->push_back("world");
  PrintDictionary(&container1);  // prints: hello\n
  PrintDictionary(container2);  // prints: world\n
}
