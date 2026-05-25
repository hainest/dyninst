#include "Symtab.h"
#include "binaryEdit.h"
#include "dynproc/dynProcess.h"
#include "patching/function.h"
#include "parsing/parse_func.h"
#include "patching/patch_block.h"
#include "patching/patch_edge.h"
#include "relocationEntry.h"

namespace st = Dyninst::SymtabAPI;

namespace Dyninst { namespace DyninstAPI {

  func_instance *patch_block::callee(std::string const &target_name) {
    if (dynamic_cast<PCProcess *>(proc())) {
      std::vector<func_instance *> pdfv;
      if (proc()->findFuncsByMangled(target_name, pdfv)) {
        obj()->setCallee(this, pdfv[0]);
        updateCallTarget(pdfv[0]);
        return pdfv[0];
      }
    }
    if (auto *bedit = dynamic_cast<BinaryEdit *>(proc())) {
      std::vector<func_instance *> pdfv;
      if (bedit->findFuncsByMangled(target_name, pdfv)) {
        obj()->setCallee(this, pdfv[0]);
        updateCallTarget(pdfv[0]);
        return pdfv[0];
      }
      for (auto *sib : bedit->getSiblings()) {
        if (sib->findFuncsByMangled(target_name, pdfv)) {
          obj()->setCallee(this, pdfv[0]);
          updateCallTarget(pdfv[0]);
          return pdfv[0];
        }
      }
    }
    return nullptr;
  }

  // callee: finds the function called by the instruction corresponding
  // to the instPoint "instr". If the function call has been bound to an
  // address, then the callee function is returned in "target" and the
  // instPoint "callee" data member is set to pt to callee's func_instance.
  // If the function has not yet been bound, then "target" is set to the
  // func_instance associated with the name of the target function (this is
  // obtained by the PLT and relocation entries in the image), and the instPoint
  // callee is not set.  If the callee function cannot be found, (ex. function
  // pointers, or other indirect calls), it returns NULL.
  // Returns NULL on error (ex. process doesn't contain this instPoint).
  func_instance *patch_block::callee() {
    // pre-computed callee via PLT
    func_instance *ret = obj()->getCallee(this);
    if (ret)
      return ret;

    // See if we've already done this
    patch_edge *tEdge = getTarget();
    if (!tEdge) {
      return NULL;
    }

    if (!tEdge->sinkEdge()) {
      func_instance *tmp = obj()->findFuncByEntry(tEdge->trg());
      if (tmp && !(tmp->ifunc()->isPLTFunction())) {
        return tmp;
      }
    }

    // Do this the hard way - an inter-module jump
    // get the target address of this function
    Address target_addr;
    bool success;
    boost::tie(success, target_addr) = llb()->callTarget();
    if (!success) {
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
      return NULL;
    }

    // get the relocation information for this image
    st::Symtab *sym = obj()->parse_img()->getObject();
    // find the target address in the list of relocationEntries
    st::relocationEntry function_binding;
    if (sym->findPltEntryByTarget(target_addr, function_binding)) {
      Address base_addr = obj()->codeBase();
      // check to see if this function has been bound yet...if the
      // PLT entry for this function has been modified by the runtime
      // linker
      func_instance *target_pdf = 0;
      if (proc()->hasBeenBound(function_binding, target_pdf, base_addr)) {
        updateCallTarget(target_pdf);
        obj()->setCalleeName(this, target_pdf->symTabName());
        obj()->setCallee(this, target_pdf);
        return target_pdf;
      }
      return callee(function_binding.name());
    } else {
      /*
       * Sometimes, the PLT address and the CFG target aren't the same
       * (e.g., Intel's CET causes this), so we just look up by name.
       */
      func_instance *f = obj()->findFuncByEntry(tEdge->trg());
      if (!f)
        return nullptr;
      return callee(f->get_name());
    }

    return NULL;
  }

}}
