#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <proxy/proxy.h>

namespace poly {

DEFINE_MEMBER_DISPATCH(At, at, std::string(int));
DEFINE_FACADE(Dictionary, At);

}  // namespace poly

void demo_print(pro::proxy<poly::Dictionary> dictionary) {
  std::cout << dictionary(1) << std::endl;
}

int main() {
  std::map<int, std::string> container1{{1, "hello"}};
  std::vector<std::string> container2{"hello", "world"};
  demo_print(&container1);  // print: hello\n
  demo_print(&container2);  // print: world\n
  return 0;
}
