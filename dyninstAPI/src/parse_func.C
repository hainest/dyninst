#include "CodeSource.h"
#include "Function.h"
#include "debug.h"
#include "image.h"
#include "image_edge.h"
#include "parse_block.h"
#include "parse_func.h"

#include <cstdio>
#include <list>
#include <set>
#include <vector>

namespace dsym = Dyninst::SymtabAPI;
namespace parse = Dyninst::ParseAPI;

#if defined(ROUGH_MEMORY_PROFILE)
int parse_func_count = 0;
#endif

parse_func::parse_func(dsym::Function *func, pdmodule *m, image *i,
                       parse::CodeObject *obj, parse::CodeRegion *reg,
                       InstructionSource *isrc, FuncSource src)
    : Function(func->getOffset(), func->getFirstSymbol()->getMangledName(), obj,
               reg, isrc),
      func_(func), mod_(m), image_(i) {
#if defined(ROUGH_MEMORY_PROFILE)
  parse_func_count++;
  if ((parse_func_count % 100) == 0)
    fprintf(stderr, "parse_func_count: %d (%d)\n", parse_func_count,
            parse_func_count * sizeof(parse_func));
#endif
  _src = src;
  func->setData(this);
}

Dyninst::Address parse_func::getOffset() const {
  return Dyninst::ParseAPI::Function::addr();
}
Dyninst::Address parse_func::getPtrOffset() const {
  return func_->getFirstSymbol()->getPtrOffset();
}
unsigned parse_func::getSymTabSize() const {
  return func_->getFirstSymbol()->getSize();
}

bool parse_func::addSymTabName(std::string name, bool isPrimary) {
  if (func_->addMangledName(name.c_str(), isPrimary)) {
    return true;
  }
  return false;
}

bool parse_func::addPrettyName(std::string name, bool isPrimary) {
  if (func_->addPrettyName(name.c_str(), isPrimary)) {
    return true;
  }
  return false;
}

bool parse_func::addTypedName(std::string name, bool isPrimary) {
  // Count this as a pretty name in function lookup...
  if (func_->addTypedName(name.c_str(), isPrimary)) {
    return true;
  }
  return false;
}

bool parse_func::isInstrumentableByFunctionName() {
  // XXXXX kludge: these functions are called by DYNINSTgetCPUtime,
  // they can't be instrumented or we would have an infinite loop
  if (prettyName() == "gethrvtime" || prettyName() == "_divdi3" ||
      prettyName() == "GetProcessTimes")
    return false;
  return true;
}

Dyninst::Address parse_func::getEndOffset() {
  if (!parsed())
    image_->analyzeIfNeeded();
  if (blocks().empty()) {
    fprintf(stderr, "error: end offset requested for empty function\n");
    return addr();
  } else {
    return extents().back()->end();
  }
}

const std::vector<image_parRegion *> &parse_func::parRegions() {
  if (!parsed())
    image_->analyzeIfNeeded();
  return parRegionsList;
}

bool parse_func::isPLTFunction() {
  return obj()->cs()->linkage().find(addr()) != obj()->cs()->linkage().end();
}

/* Returns NULL if the address is not within a block belonging to this function.
   Why do we even bother returning NULL if the address is outside of this
   function? FIXME check whether we can do away with that.
*/
void *parse_func::getPtrToInstruction(Dyninst::Address addr) const {
  // The commented-out code checks whether the address is within
  // the bytes of this function (one of its basic blocks). Instead,
  // we do a fast path and just pass the request through to the image.
  // The old tests are preserved for posterity.
  /*
  if (addr < getOffset()) return NULL;

  // XXX this call may modify the current function, so const semantics
  //     are not actually perserved
  set<Function *> stab;
  img()->findFuncs(addr,stab);

  if(!stab.empty()) {
      set<Function*>::iterator fit = stab.begin();
      for( ; fit != stab.end(); ++fit) {
          if(*fit == this)
              return obj().getPtrToInstruction(addr);
      }
  }
  return NULL;
  */
  return isrc()->getPtrToInstruction(addr);
}

parse_block *parse_func::entryBlock() {
  if (!parsed())
    image_->analyzeIfNeeded();
  return static_cast<parse_block *>(entry());
}

bool parse_func::isLeafFunc() {
  if (!parsed())
    image_->analyzeIfNeeded();

  return callEdges().empty();
}

void parse_func::addParRegion(Dyninst::Address begin, Dyninst::Address end,
                              parRegType t) {
  image_parRegion *iPar = new image_parRegion(begin, this);
  iPar->setRegionType(t);

  // when not outlined, parent func will be same as regular
  iPar->setParentFunc(this);
  iPar->setLastInsn(end);
  parRegionsList.push_back(iPar);
}

/* This function is static.
 *
 * Find the blocks that are reachable from the seed blocks
 * if the except blocks are not part of the CFG
 */
void parse_func::getReachableBlocks(const std::set<parse_block *> &exceptBlocks,
                                    const std::list<parse_block *> &seedBlocks,
                                    std::set<parse_block *> &reachBlocks) {
  using namespace ParseAPI;
  mal_printf("reachable blocks for func %lx from %lu start blocks\n", addr(),
             seedBlocks.size());

  // init visited set with seed and except blocks
  std::set<parse_block *> visited;
  visited.insert(exceptBlocks.begin(), exceptBlocks.end());
  visited.insert(seedBlocks.begin(), seedBlocks.end());

  // add seed blocks to the worklist (unless the seed is in exceptBlocks)
  std::list<parse_block *> worklist;
  for (list<parse_block *>::const_iterator sit = seedBlocks.begin();
       sit != seedBlocks.end(); sit++) {
    visited.insert(*sit);
    if (exceptBlocks.end() == exceptBlocks.find(*sit)) {
      worklist.push_back(*sit);
      reachBlocks.insert(*sit);
    }
  }

  // iterate through worklist, adding all blocks (except for
  // seedBlocks) that are reachable through target edges to the
  // reachBlocks set
  while (worklist.size()) {
    parse_block *curBlock = worklist.front();
    const Block::edgelist &outEdges = curBlock->targets();
    Block::edgelist::const_iterator tIter = outEdges.begin();
    for (; tIter != outEdges.end(); tIter++) {
      parse_block *targB = (parse_block *)(*tIter)->trg();
      if (CALL != (*tIter)->type() && false == (*tIter)->sinkEdge() &&
          visited.end() == visited.find(targB))
      // reachBlocks.end() == reachBlocks.find(targB) &&
      // exceptBlocks.end() == exceptBlocks.find(targB) &&
      // seedBlocks.end() == seedBlocks.find(targB) )
      {
        worklist.push_back(targB);
        reachBlocks.insert(targB);
        visited.insert(targB);
        mal_printf("block [%lx %lx] is reachable\n", targB->firstInsnOffset(),
                   targB->endOffset());
      }
    }
    worklist.pop_front();
  }
}
void parse_func::setinit_retstatus(ParseAPI::FuncReturnStatus rs) {
  init_retstatus_ = rs;
  if (rs > retstatus()) {
    set_retstatus(rs);
  }
}
ParseAPI::FuncReturnStatus parse_func::init_retstatus() const {
  if (ParseAPI::FuncReturnStatus::UNSET == init_retstatus_) {
    assert(
        !obj()->defensiveMode()); // should have been set for defensive binaries
    return retstatus();
  }
  if (init_retstatus_ > retstatus()) {
    return retstatus();
  }
  return init_retstatus_;
}

void parse_func::setHasWeirdInsns(bool wi) { hasWeirdInsns_ = wi; }

bool parse_func::hasUnresolvedCF() {
  if (unresolvedCF_ == UnresolvedCF::UNSET_CF) {
    for (auto *block : blocks()) {
      for (auto *edge : block->targets()) {
        if (edge->sinkEdge()) {
          if (edge->interproc()) {
            continue;
          }
          if ((edge->type() == ParseAPI::INDIRECT) ||
              (edge->type() == ParseAPI::DIRECT)) {
            unresolvedCF_ = UnresolvedCF::HAS_UNRESOLVED_CF;
            break;
          }
        }
      }
      if (unresolvedCF_ == UnresolvedCF::HAS_UNRESOLVED_CF)
        break;
    }
    if (unresolvedCF_ == UnresolvedCF::UNSET_CF)
      unresolvedCF_ = UnresolvedCF::NO_UNRESOLVED_CF;
  }
  return (unresolvedCF_ == UnresolvedCF::HAS_UNRESOLVED_CF);
}

bool parse_func::isInstrumentable() {
  if (!isInstrumentableByFunctionName() || img()->isUnlinkedObjectFile())
    return false;
  else {
    // Create instrumentation points for non-plt functions
    if (obj()->cs()->linkage().find(getOffset()) !=
        obj()->cs()->linkage().end()) {
      return false;
    }
  }

  if (hasUnresolvedCF()) {
    return false;
  }
  return true;
}

typedef parse_func *ifuncPtr;

struct ifuncCmp {
  int operator()(const ifuncPtr &f1, const ifuncPtr &f2) const {
    if (f1->getOffset() > f2->getOffset())
      return 1;
    if (f1->getOffset() < f2->getOffset())
      return -1;
    return 0;
  }
};
