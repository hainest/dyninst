#include <iostream>

#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emit-x86.h"

int main() {
  constexpr auto num_bytes = 15;
  codeGen gen(num_bytes);
  EmitterIA32Dyn emitter;
  emitter.emitLoad(REGNUM_EAX, 0x12345678, 4, gen);
  
  auto *buf = static_cast<char const*>(gen.start_ptr());
  std::cout << "0x";
  for(auto i=0; i<num_bytes; i++) {
    std::cout << std::hex << static_cast<int>(buf[i]);
  }
}
