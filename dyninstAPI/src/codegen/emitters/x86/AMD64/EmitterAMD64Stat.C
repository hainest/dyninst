#include "codegen/emitters/x86/AMD64/EmitterAMD64Stat.h"
#include "codegen/emitters/x86/generators.h"
#include "function.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterAMD64Stat::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
    // fprintf(stdout, "at emitCallInstruction: callee=%s\n", callee->prettyName().c_str());

    AddressSpace *addrSpace = gen.addrSpace();
    Address dest;

    // find func_instance reference in address space
    // (refresh func_map)
    std::vector<func_instance *> funcs;
    addrSpace->findFuncsByAll(callee->prettyName(), funcs);

    // test to see if callee is in a shared module
    assert(gen.func());
    if(gen.func()->obj() != callee->obj()) {
      emitPLTCall(callee, gen);
    } else {
      dest = callee->addr();
      signed long disp = dest - (gen.currAddr() + 5);
      int disp_i = (int)disp;
      assert(disp == (signed long)disp_i);
      x86::emitCallRel32(disp_i, gen);
      return true;
    }
    return true;
  }

  bool EmitterAMD64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    // create or retrieve jump slot
    Address dest = getInterModuleFuncAddr(callee, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xFF);
    // Note: this is a combination of 00 (MOD), 100 (opcode extension), and 101
    // (disp32)
    append_memory_as_byte(insn, 0x25);
    int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
    assert(numeric_limits<int32_t>::lowest() <= offset &&
           offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
    append_memory_as(insn, static_cast<int32_t>(offset));
    SET_PTR(insn, gen);
    return true;
  }

  bool EmitterAMD64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    // create or retrieve jump slot
    Address dest = getInterModuleFuncAddr(callee, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xFF);
    append_memory_as_byte(insn, 0x15);
    int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
    assert(numeric_limits<int32_t>::lowest() <= offset &&
           offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
    append_memory_as(insn, static_cast<int32_t>(offset));
    SET_PTR(insn, gen);
    return true;
  }

}}
