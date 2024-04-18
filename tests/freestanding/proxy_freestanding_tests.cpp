#include "proxy.h"

unsigned GetHash(int v) { return static_cast<unsigned>(v + 3) * 31; }
unsigned GetHash(double v) { return static_cast<unsigned>(v * v + 5) * 87; }
unsigned GetHash(const char* v) {
  unsigned result = 91u;
  for (int i = 0; v[i]; ++i) {
    result = result * 47u + v[i];
  }
  return result;
}
unsigned GetDefaultHash() { return -1; }

namespace spec {

PRO_DEF_FREE_DISPATCH_WITH_DEFAULT(GetHash, ::GetHash, ::GetDefaultHash, unsigned());
PRO_DEF_FACADE(Hashable, GetHash);

}  // namespace spec

extern "C" int main() {
  int i = 123;
  double d = 3.14159;
  const char* s = "lalala";
  std::tuple<int, double> t{11, 22};
  pro::proxy<spec::Hashable> p;
  p = &i;
  if (p() != GetHash(i)) {
    return 1;
  }
  p = &d;
  if (p() != GetHash(d)) {
    return 1;
  }
  p = pro::make_proxy_inplace<spec::Hashable>(s);
  if (p() != GetHash(s)) {
    return 1;
  }
  p = &t;
  if (p() != GetDefaultHash()) {
    return 1;
  }
  return 0;
}

extern "C" void _start() {
#if defined(__x86_64__)
  asm(
    "call main\n"
    "mov %eax, %edi\n"
    "mov $60, %eax\n"
    "syscall"
  );
#elif defined(__aarch64__)
  asm(
    "bl main\n"
    "mov w0, w0\n"
    "mov w8, #93\n"
    "svc #0\n"
  );
#elif defined(__riscv)
  asm(
    "call main\n"
    "mv a1, a0\n"
    "li a7, 93\n"
    "ecall\n"
  );
#else
#error "Unknown architecture"
#endif
}
