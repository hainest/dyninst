#include "ASTs/codeGenAST.h"
#include "Instruction.h"
#include "Register.h"
#include "addressSpace.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Dyn.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Stat.h"
#include "registerSpace/registerSpace.h"
#include "registers/ppc64_regs.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

bool AddressSpace::getDynamicCallSiteArgs(di::Instruction i, Address addr,
                                          std::vector<da::codeGenASTPtr> &args) {

  static di::RegisterAST::Ptr ctr(new di::RegisterAST(ppc64::ctr));
  static di::RegisterAST::Ptr lr(new di::RegisterAST(ppc64::lr));

  Dyninst::Register branch_target = registerSpace::ignored;

  // Is this a branch conditional link register (BCLR)
  // BCLR uses the xlform (6,5,5,5,10,1)
  for (di::Instruction::cftConstIter curCFT = i.cft_begin(); curCFT != i.cft_end();
       ++curCFT) {
    if (curCFT->target->isUsed(ctr)) {
      branch_target = registerSpace::ctr;
      break;
    } else if (curCFT->target->isUsed(lr)) {
      fprintf(stderr, "setting lr\n");
      branch_target = registerSpace::lr;
      break;
    }
  }
  if (branch_target != registerSpace::ignored) {
    // Where we're jumping to (link register, count register)
    args.push_back(da::operandAST::origRegister((void *)(long)branch_target));

    // Where we are now
    args.push_back(da::operandAST::Constant((void *)addr));

    return true;
  } else {
    return false;
  }
}

Emitter *AddressSpace::getEmitter() {
  static Dyninst::DyninstAPI::EmitterPowerPC32Dyn emitter32Dyn;
  static Dyninst::DyninstAPI::EmitterPowerPC64Dyn emitter64Dyn;
  static Dyninst::DyninstAPI::EmitterPowerPC32Stat emitter32Stat;
  static Dyninst::DyninstAPI::EmitterPowerPC64Stat emitter64Stat;

  if (getAddressWidth() == 8) {
    if (proc()) {
      return &emitter64Dyn;
    } else
      return &emitter64Stat;
  }
  if (proc())
    return &emitter32Dyn;
  else
    return &emitter32Stat;
}
