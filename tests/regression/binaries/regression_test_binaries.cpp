void foo1() {}

void foo2() {}

void FOO3() {}

namespace Dyninst { namespace RegressionTests {

  void test1() {}

}}

#ifdef ADD_INTERP
const char __invoke_dynamic_linker__[] __attribute__((section(".interp"))) =
    "/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2";
#endif

#ifdef HAS_MAIN
int main() {}
#endif
