#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <proxy/proxy.h>

namespace spec {

PRO_DEF_MEMBER_DISPATCH(at, std::string(int));
PRO_DEF_FACADE(Dictionary, at);

}  // namespace spec

void demo_print(pro::proxy<spec::Dictionary> dictionary) {
  std::cout << dictionary.at(1) << std::endl;
}

int main() {
  std::map<int, std::string> container1{{1, "hello"}};
  std::vector<std::string> container2{"hello", "world"};
  demo_print(&container1);  // print: hello\n
  demo_print(&container2);  // print: world\n
  return 0;
}
