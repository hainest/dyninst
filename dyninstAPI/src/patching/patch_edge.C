#include "patch_edge.h"

namespace Dyninst { namespace DyninstAPI {

  patch_edge::patch_edge(ParseAPI::Edge *edge, block_instance *source,
                         block_instance *target)
      : PatchAPI::PatchEdge(edge, source, target) {
  }

  // For forked process
  patch_edge::patch_edge(const patch_edge *parent, mapped_object *child)
      : PatchAPI::PatchEdge(
            parent, parent->src_ ? child->findBlock(parent->src()->llb()) : NULL,
            parent->trg_ ? child->findBlock(parent->trg()->llb()) : NULL) {
  }

  AddressSpace *patch_edge::proc() {
    return src()->proc();
  }

  block_instance *patch_edge::src() const {
    return SCAST_BI(src_);
  }

  block_instance *patch_edge::trg() const {
    return SCAST_BI(trg_);
  }

}}
