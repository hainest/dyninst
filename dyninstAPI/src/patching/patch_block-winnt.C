#include "Instruction.h"
#include "addressSpace.h"
#include "function.h"
#include "patching/patch_block.h"
#include "patching/patch_edge.h"
#include "registers/x86_regs.h"

#include <cassert>

namespace Dyninst { namespace DyninstAPI {

  bool thunkILT(patch_edge *edge, AddressSpace *proc, func_instance *&ret) {
    assert(!edge->sinkEdge());
    // We have a direct call but don't yet know the callee.
    // This may be because we didn't see a symbol for a
    // thunk in the ILT.
    // (I.e., the call is 'call @ILT+<smallconstant>'
    // and at @ILT+<smallconstant> there is a 'jmp <realfuncaddr>'
    // instruction.
    //
    // We consider the callee to be the real function that
    // is eventually called.
    // In CFG terms, this is:
    // We're calling a function that satisfies the following:
    //  1) 1 block long
    //  2) 1 instruction long
    //  3) Has a direct (?) edge to another function entry point.

    // get the target address of the call

    func_instance *cFunc = proc->findFuncByEntry(edge->trg());
    if (cFunc == NULL)
      return false;

    // 1)
    if (cFunc->blocks().size() > 1)
      return false;

    // 2)
    patch_block *cBlock = cFunc->entryBlock();
    patch_block::Insns cInsns;
    cBlock->getInsns(cInsns);
    if (cInsns.size() > 1)
      return false;

    // 3)
    patch_edge *cEdge = cBlock->getTarget();
    if (!cEdge)
      return false;
    if (cEdge->sinkEdge())
      return false;

    patch_block *block = cEdge->trg();
    if (!block)
      return false;
    func_instance *func = proc->findFuncByEntry(block);
    if (!func)
      return false;

    ret = func;
    return true;
  }

  // findCallee: returns false unless callee is already set in instPoint
  // dynamic linking not implemented on this platform
  func_instance *patch_block::callee() {
    /* Unlike Linux, we do some interpretation here. Windows uses a common idiom of the
     * following. Source: void foo() { bar(); } void bar() { ... } Which turns into foo:
     *   call stub
     * bar:
     *   ...
     * stub:
     *   jmp bar
     *
     * Since we're interpreting callees, we want to track this down
     * and represent the callee as bar, not stub.
     */
    if (!containsCall()) {
      return NULL;
    }

    patch_edge *tEdge = getTarget();
    if (!tEdge)
      return NULL;

    // Otherwise use the target function...
    if (!tEdge->sinkEdge()) {
      func_instance *ret;
      // If we're calling a 1-instruction function that branches to a known entry point,
      // elide that...
      if (thunkILT(tEdge, proc(), ret)) {
        return ret;
      }
      return tEdge->trg()->obj()->findFuncByEntry(tEdge->trg());
    }

    // An call that uses an indirect call instruction could be one
    // of three things:
    //
    // 1. A call to a function within the same executable or DLL.  In
    //    this case, it could be a call through a function pointer in
    //    memory or a register, or some other type of call.
    //
    // 2. A call to a function in an implicitly-loaded DLL.  These are
    //    indirect calls through an entry in the object's Import Address
    //    Table (IAT). An IAT entry is set to the address of the target
    //    when the DLL is loaded at process creation.
    //
    // 3. A call to a function in a delay-loaded DLL.  Like calls to
    //    implicitly-loaded DLLs, these are calls through an entry in the
    //    IAT.  However, for delay-loaded DLLs the IAT entry initially
    //    holds the address of a short code sequence that loads the DLL,
    //    looks up the target function, patches the IAT entry with the
    //    address of the target function, and finally executes the target
    //    function.  After the first call, the IAT entry has been patched
    //    and subsequent calls look the same as calls into an
    //    implicitly-loaded DLL.
    //
    // Figure out what type of indirect call instruction this is
    //
    using namespace Dyninst::InstructionAPI;
    InstructionAPI::Instruction::Ptr insn = getInsn(last());
    if (insn && (insn->getCategory() == c_CallInsn)) {
      Expression::Ptr cft = insn->getControlFlowTarget();
      static Expression *theIP = new RegisterAST(x86::eip);
      cft->bind(theIP, Result(u64, last()));
      Result r = cft->eval();
      if (r.defined) {
        Address funcPtrAddress = r.convert<Address>();
        assert(funcPtrAddress != ADDR_NULL);

        // obtain the target address from memory if it is available
        Address targetAddr = ADDR_NULL;
        if (insn->readsMemory()) {
          proc()->readDataSpace((const void *)funcPtrAddress, sizeof(Address),
                                &targetAddr, true);
        } else { // this is not an indirect call at all, but a call to
                 // an uninitialized or invalid address
          targetAddr = funcPtrAddress;
        }
        if (targetAddr != ADDR_NULL) {
          // see whether we already know anything about the target
          // this may be the case with implicitly-loaded and delay-loaded
          // DLLs, and it is possible with other types of indirect calls
          func_instance *target = proc()->findFuncByEntry(targetAddr);
          if (target) {
            updateCallTarget(target);
            return target;
          }
        }
      }
    }
    return NULL;
  }

}}
