/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTER_AARCH64_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTER_AARCH64_H

#include "emitter.h"
#include "dyn_register.h"
#include "codegen.h"

class image_variable;

class EmitterAARCH64 : public Emitter {
public:
    virtual ~EmitterAARCH64() {}

    virtual codeBufIndex_t emitIf(Dyninst::Register, Dyninst::Register, RegControl, codeGen &);

    virtual void emitOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &);

    virtual void emitOpImm(unsigned, unsigned, Dyninst::Register, Dyninst::Register, Dyninst::RegValue,
                           codeGen &) { assert(0); }

    virtual void emitRelOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool);

    virtual void emitRelOpImm(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &, bool);

    virtual void emitDiv(Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool) { assert(0); }

    virtual void emitTimesImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &) { assert(0); }

    virtual void emitDivImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &, bool) { assert(0); }

    virtual void emitLoad(Dyninst::Register, Dyninst::Address, int, codeGen &);

    virtual void emitLoadConst(Dyninst::Register, Dyninst::Address, codeGen &);

    virtual void emitLoadIndir(Dyninst::Register, Dyninst::Register, int, codeGen &);

    virtual bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &);

    virtual bool emitLoadRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &);

    virtual void
    emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen,
                   Dyninst::Address offset);

    virtual void emitLoadFrameAddr(Dyninst::Register, Dyninst::Address, codeGen &) { assert(0); }

    // These implicitly use the stored original/non-inst value
    virtual void emitLoadOrigFrameRelative(Dyninst::Register, Dyninst::Address, codeGen &) { assert(0); }

    virtual void emitLoadOrigRegRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &, bool);

    virtual void emitLoadOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &);

    virtual void emitStore(Dyninst::Address, Dyninst::Register, int, codeGen &);

    virtual void emitStoreIndir(Dyninst::Register, Dyninst::Register, int, codeGen &);

    virtual void emitStoreFrameRelative(Dyninst::Address, Dyninst::Register, Dyninst::Register, int, codeGen &) { assert(0); }

    virtual void emitStoreRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &);

    virtual void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen);


    virtual void emitStoreOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &) { assert(0); }

    virtual bool emitMoveRegToReg(Dyninst::Register, Dyninst::Register, codeGen &) {
        assert(0);
        return 0;
    }

    virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);

    virtual Dyninst::Address emitMovePCToReg(Dyninst::Register, codeGen &gen);

    // This one we actually use now.
    virtual Dyninst::Register emitCall(opCode, codeGen &, const std::vector <AstNodePtr> &,
                              bool, func_instance *);

    virtual void emitGetRetVal(Dyninst::Register, bool, codeGen &) { assert(0); }

    virtual void emitGetRetAddr(Dyninst::Register, codeGen &) { assert(0); }

    virtual void emitGetParam(Dyninst::Register, Dyninst::Register, instPoint::Type, opCode, bool, codeGen &);

    virtual void emitASload(int, int, int, long, Dyninst::Register, int, codeGen &) { assert(0); }

    virtual void emitCSload(int, int, int, long, Dyninst::Register, codeGen &) { assert(0); }

    virtual void emitPushFlags(codeGen &) { assert(0); }

    virtual void emitRestoreFlags(codeGen &, unsigned) { assert(0); }

    // Built-in offset...
    virtual void emitRestoreFlagsFromStackSlot(codeGen &) { assert(0); }

    virtual bool emitBTSaves(baseTramp *, codeGen &) {
        assert(0);
        return true;
    }

    virtual bool emitBTRestores(baseTramp *, codeGen &) {
        assert(0);
        return true;
    }

    virtual void emitStoreImm(Dyninst::Address, int, codeGen &, bool) { assert(0); }

    virtual void emitAddSignedImm(Dyninst::Address, int, codeGen &, bool) { assert(0); }

    virtual bool emitPush(codeGen &, Dyninst::Register) {
        assert(0);
        return true;
    }

    virtual bool emitPop(codeGen &, Dyninst::Register) {
        assert(0);
        return true;
    }

    virtual bool emitAdjustStackPointer(int, codeGen &) {
        assert(0);
        return true;
    }

    virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);

protected:
    virtual bool emitCallInstruction(codeGen &, func_instance *,
                                     bool, Dyninst::Address);

    virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool,
                                         func_instance *);
};

#endif

