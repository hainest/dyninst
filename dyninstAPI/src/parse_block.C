#include "InstructionDecoder.h"
#include "debug.h"
#include "image.h"
#include "parse-cfg.h"
#include "parse_block.h"

#include <vector>

namespace parse = Dyninst::ParseAPI;

#if defined(ROUGH_MEMORY_PROFILE)
#include <cstdio>
static int parse_block_count = 0;
#endif

Dyninst::Address parse_block::firstInsnOffset() const {
  return ParseAPI::Block::start();
}
Dyninst::Address parse_block::lastInsnOffset() const {
  return ParseAPI::Block::lastInsnAddr();
}
Dyninst::Address parse_block::endOffset() const {
  return ParseAPI::Block::end();
}
Dyninst::Address parse_block::getSize() const {
  return ParseAPI::Block::size();
}

parse_block::parse_block(parse_func *func, parse::CodeRegion *reg,
                         Dyninst::Address firstOffset)
    : parse::Block(func->obj(), reg, firstOffset, func) {

  // basic block IDs are unique within images.
  blockNumber_ = func->img()->getNextBlockID();

#if defined(ROUGH_MEMORY_PROFILE)
  parse_block_count++;
  if ((parse_block_count % 100) == 0)
    fprintf(stderr, "parse_block_count: %d (%d)\n", parse_block_count,
            parse_block_count * sizeof(parse_block));
#endif
}

void parse_block::debugPrint() {
  // no looping if we're not printing anything
  if (!dyn_debug_parsing)
    return;

  parsing_printf("Block %d: starts 0x%lx, last 0x%lx, end 0x%lx\n",
                 blockNumber_, start(), lastInsnAddr(), end());

  parsing_printf("  Sources:\n");
  const Block::edgelist &srcs = sources();
  Block::edgelist::const_iterator sit = srcs.begin();
  unsigned s = 0;
  for (; sit != srcs.end(); ++sit) {
    parse_block *src = static_cast<parse_block *>((*sit)->src());
    parsing_printf("    %u: block %d (%s)\n", s, src->blockNumber_,
                   static_cast<image_edge *>(*sit)->getTypeString());
    ++s;
  }
  parsing_printf("  Targets:\n");
  const Block::edgelist &trgs = sources();
  Block::edgelist::const_iterator tit = trgs.begin();
  unsigned t = 0;
  for (; tit != trgs.end(); ++tit) {
    parse_block *trg = static_cast<parse_block *>((*tit)->trg());
    parsing_printf("    %u: block %d (%s)\n", t, trg->blockNumber_,
                   static_cast<image_edge *>(*tit)->getTypeString());
    ++t;
  }
}

void *parse_block::getPtrToInstruction(Address addr) const {
  if (addr < start())
    return NULL;
  if (addr >= end())
    return NULL;
  // XXX all potential parent functions have the same image
  return region()->getPtrToInstruction(addr);
}

bool parse_block::isCallBlock() {
  const parse::Block::edgelist &trgs = targets();
  if (!trgs.empty()) {
    for (Block::edgelist::const_iterator eit = trgs.begin(); eit != trgs.end();
         eit++) {
      if ((*eit)->type() == parse::CALL) {
        return true;
      }
    }
  }
  return false;
}
bool parse_block::isIndirectTailCallBlock() {
  const parse::Block::edgelist &trgs = targets();
  if (!trgs.empty()) {
    for (parse::Block::edgelist::const_iterator eit = trgs.begin();
         eit != trgs.end(); eit++) {
      if ((*eit)->type() == parse::INDIRECT && (*eit)->interproc()) {
        return true;
      }
    }
  }
  return false;
}
image *parse_block::img() {
  std::vector<parse::Function *> funcs;
  getFuncs(funcs);
  return static_cast<parse_func *>(funcs[0])->img();
}

parse_func *parse_block::getEntryFunc() const {
  parse_func *ret =
      static_cast<parse_func *>(obj()->findFuncByEntry(region(), start()));

  // sanity check
  if (ret && ret->entryBlock() != this) {
    parsing_printf("[%s:%d] anomaly: block [%lx,%lx) is not entry for "
                   "func at %lx\n",
                   FILE__, __LINE__, start(), end(), ret->addr());
  }
  return ret;
}

bool parse_block::isEntryBlock(parse_func *f) const {
  return f->entryBlock() == this;
}

/*
 * True if the block has a return edge, or a call that does
 * not return (i.e., a tail call or non-returning call)
 */
bool parse_block::isExitBlock() {
  const parse::Block::edgelist &trgs = targets();
  if (trgs.empty()) {
    return false;
  }

  parse::Edge *e = *trgs.begin();
  if (e->type() == parse::RET) {
    return true;
  }

  if (!e->interproc()) {
    return false;
  }

  if (e->type() == parse::CALL && trgs.size() > 1) {
    // there's a CALL edge and at least one other edge,
    // it's an exit block if there is no CALL_FT edge
    for (Block::edgelist::const_iterator eit = ++(trgs.begin());
         eit != trgs.end(); eit++) {
      if ((*eit)->type() == parse::CALL_FT && !(*eit)->sinkEdge()) {
        return false;
      }
    }
  }
  return true;
}

void parse_block::setUnresolvedCF(bool newVal) { unresolvedCF_ = newVal; }

parse_func *parse_block::getCallee() {
  for (edgelist::const_iterator iter = targets().begin();
       iter != targets().end(); ++iter) {
    if ((*iter)->type() == ParseAPI::CALL) {
      parse_block *t = static_cast<parse_block *>((*iter)->trg());
      return t->getEntryFunc();
    }
  }
  return NULL;
}

std::pair<bool, Address> parse_block::callTarget() {
  using namespace InstructionAPI;
  Offset off = lastInsnOffset();
  const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
  if (ptr == NULL)
    return std::make_pair(false, 0);
  InstructionDecoder d(ptr, endOffset() - lastInsnOffset(),
                       obj()->cs()->getArch());
  Instruction insn = d.decode();

  // Bind PC to that insn
  // We should build a free function to do this...

  Expression::Ptr cft = insn.getControlFlowTarget();
  if (cft) {
    Expression::Ptr pc(
        new RegisterAST(MachRegister::getPC(obj()->cs()->getArch())));
    cft->bind(pc.get(), Result(u64, lastInsnAddr()));
    Result res = cft->eval();
    if (!res.defined)
      return std::make_pair(false, 0);

    return std::make_pair(true, res.convert<Address>());
  }
  return std::make_pair(false, 0);
}

void parse_block::getInsns(Insns &insns, Address base) {
  using namespace InstructionAPI;
  Offset off = firstInsnOffset();
  const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
  if (ptr == NULL)
    return;

  InstructionDecoder d(ptr, getSize(), obj()->cs()->getArch());

  while (off < endOffset()) {
    Instruction insn = d.decode();

    insns[off + base] = insn;
    off += insn.size();
  }
}
