#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/generators/xmm.h"
#include "registerSpace/registerSlot.h"
#include "registerSpace/registerSpace.h"
#include "unaligned_memory_access.h"

namespace Dyninst { namespace DyninstAPI { namespace AMD64 {

  void emitXMMRegsSaveRestore(codeGen &gen, bool isRestore) {
    GET_PTR(insn, gen);
    for(int reg = 0; reg <= 7; ++reg) {
      registerSlot *r = (*gen.rs())[(REGNUM_XMM0 + reg)];
      if(r && r->liveState == registerSlot::dead) {
        continue;
      }
      unsigned char offset = reg * 16;
      append_memory_as_byte(insn, 0x66);
      append_memory_as_byte(insn, 0x0f);
      // 6f to save, 7f to restore
      if(isRestore) {
        append_memory_as_byte(insn, 0x6f);
      } else {
        append_memory_as_byte(insn, 0x7f);
      }

      if(reg == 0) {
        append_memory_as_byte(insn, 0x00);
      } else {
        unsigned char modrm = 0x40 + (0x8 * reg);
        append_memory_as_byte(insn, modrm);
        append_memory_as_byte(insn, offset);
      }
    }
    SET_PTR(insn, gen);
  }

}}}
