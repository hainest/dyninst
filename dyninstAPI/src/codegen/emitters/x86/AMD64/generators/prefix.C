#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/generators/prefix.h"
#include "codegen/emitters/x86/generators.h"
#include "registerSpace/registerSlot.h"
#include "unaligned_memory_access.h"

namespace Dyninst { namespace DyninstAPI { namespace AMD64 {

  void emitRex(bool is_64, Register *r, Register *x, Register *b, codeGen &gen) {
    unsigned char rex = 0x40;

    // need rex for 64-bit ops in most cases
    if(is_64) {
      rex |= 0x08;
    }

    // need rex for use of new registers
    // if a new register is used, we mask off the high bit before
    // returning since we account for it in the rex prefix

    // "R" register - extension to ModRM reg field
    if(r && (*r & 0x08)) {
      rex |= 0x04;
      *r = r->getId() & 0x07;
    }

    // "X" register - extension to SIB index field
    if(x && (*x & 0x08)) {
      rex |= 0x02;
      *x = x->getId() & 0x07;
    }

    // "B" register - extension to ModRM r/m field, SIB base field,
    // or opcode reg field
    if(b && (*b & 0x08)) {
      rex |= 0x01;
      *b = b->getId() & 0x07;
    }

    // emit the rex, if needed
    // (note that some other weird cases not covered here
    //  need a "blank" rex, like using %sil or %dil)
    if(rex & 0x0f) {
      x86::emitSimpleInsn(rex, gen);
    }
  }

}}}
