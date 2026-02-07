#include "CFG.h"
#include "image_edge.h"
#include "parse_block.h"

parse_block *image_edge::src() const {
  return static_cast<parse_block *>(Dyninst::ParseAPI::Edge::src());
}

parse_block *image_edge::trg() const {
  return static_cast<parse_block *>(Dyninst::ParseAPI::Edge::trg());
}

const char *image_edge::getTypeString() {
  switch (type()) {
  case Dyninst::ParseAPI::CALL:
    return "CALL";
  case Dyninst::ParseAPI::COND_TAKEN:
    return "COND BRANCH - TAKEN";
  case Dyninst::ParseAPI::COND_NOT_TAKEN:
    return "COND BRANCH - NOT TAKEN";
  case Dyninst::ParseAPI::INDIRECT:
    return "INDIRECT BRANCH";
  case Dyninst::ParseAPI::DIRECT:
    return "UNCOND BRANCH";
  case Dyninst::ParseAPI::FALLTHROUGH:
    return "FALLTHROUGH";
  case Dyninst::ParseAPI::CATCH:
    return "CATCH";
  case Dyninst::ParseAPI::CALL_FT:
    return "POST-CALL FALLTHROUGH";
  case Dyninst::ParseAPI::RET:
    return "RETURN";
  case Dyninst::ParseAPI::NOEDGE:
    return "NO EDGE";
  case Dyninst::ParseAPI::_edgetype_end_:
    break;
  }
  return "ERROR UNKNOWN";
}
