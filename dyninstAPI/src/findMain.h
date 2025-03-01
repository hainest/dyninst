#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "CFG.h"
#include "debug.h"
#include "DynAST.h"
#include "dyntypes.h"
#include "Function.h"
#include "SymEval.h"
#include "Symtab.h"

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;
  namespace df = Dyninst::DataflowAPI;

  namespace ppc {
    Dyninst::Address find_main_by_toc(st::Symtab*, pa::Function*, pa::Block*);
  }

  class FindMainVisitor final : public Dyninst::ASTVisitor {
    using ASTVisitor::visit;

  public:
    bool resolved{false};
    bool hardFault{false};
    Dyninst::Address target{};

    Dyninst::AST::Ptr visit(df::RoseAST* r) override {
      Dyninst::AST::Children newKids;
      for(unsigned i = 0; i < r->numChildren(); i++) {
        newKids.push_back(r->child(i)->accept(this));
      }

      if(r->val().op == df::ROSEOperation::addOp) {
        auto const_ast = Dyninst::AST::V_ConstantAST;
        assert(newKids.size() == 2);
        if(newKids[0]->getID() == const_ast && newKids[1]->getID() == const_ast) {
          auto c1 = df::ConstantAST::convert(newKids[0]);
          auto c2 = df::ConstantAST::convert(newKids[1]);
          if(!hardFault) {
            target = c1->val().val + c2->val().val;
            resolved = true;
          }
          return df::ConstantAST::create(df::Constant(c1->val().val + c2->val().val));
        }
      } else {
        startup_printf("%s[%d] unhandled FindMainVisitor operation %d\n", FILE__, __LINE__, r->val().op);
      }

      return df::RoseAST::create(r->val(), newKids);
    }

    ASTPtr visit(df::ConstantAST* c) override {
      /* We can only handle constant values */
      if(!target && !hardFault) {
        resolved = true;
        target = c->val().val;
      }

      return c->ptr();
    }

    ASTPtr visit(df::VariableAST* v) override {
      /* If we visit a variable node, we can't do any analysis */
      hardFault = true;
      resolved = false;
      target = 0;
      return v->ptr();
    }
  };

}}

#endif
