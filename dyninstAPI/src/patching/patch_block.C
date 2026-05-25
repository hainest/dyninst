#include "CFG.h"
#include "PatchModifier.h"
#include "debug.h"
#include "function.h"
#include "patch_block.h"

namespace Dyninst { namespace DyninstAPI {

  patch_block::patch_block(ParseAPI::Block *ib, mapped_object *obj)
      : PatchBlock(ib, obj), _ignorePowerPreamble(false) {
    // We create edges lazily
  }

  // Fork constructor
  patch_block::patch_block(const patch_block *parent, mapped_object *childObj)
      : PatchBlock(parent, childObj), _ignorePowerPreamble(parent->_ignorePowerPreamble) {
    // We also need to copy edges.
    // Thing is, those blocks may not exist yet...
    // So we wait, and do edges after all blocks have
    // been created
  }

  AddressSpace *patch_block::addrSpace() const {
    return obj()->proc();
  }

  Dyninst::DyninstAPI::patch_edge *patch_block::getFallthrough() {
    for (edgelist::const_iterator iter = targets().begin(); iter != targets().end();
         ++iter) {
      if ((*iter)->type() == ParseAPI::FALLTHROUGH ||
          (*iter)->type() == ParseAPI::CALL_FT ||
          (*iter)->type() == ParseAPI::COND_NOT_TAKEN) {
        return SCAST_EI(*iter);
      }
    }
    return NULL;
  }

  patch_block *patch_block::getFallthroughBlock() {
    Dyninst::DyninstAPI::patch_edge *ft = getFallthrough();
    if (ft && !ft->sinkEdge())
      return ft->trg();
    else
      return NULL;
  }

  Dyninst::DyninstAPI::patch_edge *patch_block::getTarget() {
    for (edgelist::const_iterator iter = targets().begin(); iter != targets().end();
         ++iter) {
      if ((*iter)->type() == ParseAPI::CALL || (*iter)->type() == ParseAPI::DIRECT ||
          (*iter)->type() == ParseAPI::COND_TAKEN) {
        return SCAST_EI(*iter);
      }
    }
    return NULL;
  }

  int patch_block::id() const {
    return llb()->id();
  }

  Address patch_block::GetBlockStartingAddress() {
    return llb()->start();
  }

  using namespace Dyninst::Relocation;
  void patch_block::triggerModified() {
    // KEVINTODO: implement this: remove block from Relocation info caching...
    // PCSensitiveTransformer::invalidateCache(this);
  }

  void patch_block::setNotAbruptEnd() {
    llb()->setAbruptEnd(false);
    vector<func_instance *> funcs;
    getFuncs(std::back_inserter(funcs));
    for (vector<func_instance *>::iterator fit = funcs.begin(); fit != funcs.end();
         fit++) {
      (*fit)->removeAbruptEnd(this);
    }
  }

  std::string patch_block::calleeName() {
    // How the heck do we do this again?
    return obj()->getCalleeName(this);
  }

  void patch_block::updateCallTarget(func_instance *func) {
    // Update a sink-typed call edge to
    // have an inter-module target
    //
    // Preserving original behavior on sink edges only
    //
    Dyninst::DyninstAPI::patch_edge *e = getTarget();
    if (e && e->sinkEdge()) {
      PatchAPI::PatchModifier::redirect(e, func->entryBlock());
    } else {
      mal_printf("WARNING: tried to update the call target of a block "
                 "[%lx %lx) with a non-sink target %lx to %lx %s[%d]\n",
                 start(), end(), e->trg()->start(), func->addr(), FILE__, __LINE__);
    }
  }

  func_instance *patch_block::entryOfFunc() const {
    parse_block *b = SCAST_PB(llb());
    parse_func *e = b->getEntryFunc();
    if (!e)
      return NULL;
    return obj()->findFunction(e);
  }

  bool patch_block::isFuncExit() const {
    parse_block *b = SCAST_PB(llb());
    return b->isExitBlock();
  }

  func_instance *patch_block::findFunction(ParseAPI::Function *p) {
    return obj()->findFunction(p);
  }

  void *patch_block::getPtrToInstruction(Address addr) const {
    if (addr < start())
      return NULL;
    if (addr > end())
      return NULL;
    return obj()->getPtrToInstruction(addr);
  }

  void patch_block::markModified() {
    std::vector<PatchAPI::PatchFunction *> funcs;
    getFuncs(std::back_inserter(funcs));
    for (unsigned i = 0; i < funcs.size(); ++i) {
      funcs[i]->markModified();
    }
  }

}}
