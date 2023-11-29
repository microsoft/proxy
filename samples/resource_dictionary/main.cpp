#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <proxy/proxy.h>

DEFINE_MEMBER_DISPATCH(at, at, std::string(int));
DEFINE_FACADE(resource_dictionary, at);

void demo_print(pro::proxy<resource_dictionary> dictionary) {
  std::cout << dictionary(1) << std::endl;
}

int main() {
  std::map<int, std::string> container1{{1, "hello"}};
  std::vector<std::string> container2{"hello", "world"};
  demo_print(&container1);  // print: hello\n
  demo_print(&container2);  // print: world\n
  return 0;
}
