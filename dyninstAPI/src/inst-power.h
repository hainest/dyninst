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

#ifndef INST_POWER_H
#define INST_POWER_H

#include <stdint.h>


#define DEAD_REG              0
#define LIVE_REG              1
#define LIVE_UNCLOBBERED_REG  2
#define LIVE_CLOBBERED_REG    3

#define GPRSIZE_32            4
#define GPRSIZE_64            8
#define FPRSIZE               16

#define REG_SP		      1		
#define REG_TOC               2
#define REG_GUARD_ADDR        5
#define REG_GUARD_VALUE       6
#define REG_GUARD_OFFSET      6

#define REG_COST_ADDR         5
#define REG_COST_VALUE        6

#define REG_SCRATCH          10

#define REG_MT_POS           12
#define NUM_INSN_MT_PREAMBLE 26

#define STACKSKIP          512

#define ALIGN_QUADWORD(x)  ( ((x) + 0xf) & ~0xf )

#define GPRSAVE_32  (32*4)
#define GPRSAVE_64  (32*8)
#define FPRSAVE     (14*8)
#define VECSAVE     (33*16)

#define SPRSAVE_32  (6*4+8)
#define SPRSAVE_64  (6*8+8)
#define FUNCSAVE_32 (32*4)
#define FUNCSAVE_64 (32*8)
#define FUNCARGS_32 (16*4)
#define FUNCARGS_64 (16*8)
#define LINKAREA_32 (6*4)
#define LINKAREA_64 (6*8)

#if defined(os_linux)
#define PARAM_OFFSET(mutatee_address_width)                         \
        (                                                           \
            ((mutatee_address_width) == sizeof(uint64_t))           \
            ? (   /* 64-bit ELF PowerPC Linux                   */  \
                  sizeof(uint64_t) +  /* TOC save doubleword    */  \
                  sizeof(uint64_t) +  /* LR save doublewordd    */  \
                  sizeof(uint32_t) +  /* Reserved word          */  \
                  sizeof(uint32_t) +  /* CR save word           */  \
                  sizeof(uint64_t)    /* Stack frame back chain */  \
              )                                                     \
            : (   /* 32-bit ELF PowerPC Linux                   */  \
                  sizeof(uint32_t) +  /* LR save                */  \
                  sizeof(uint32_t)    /* Stack frame back chain */  \
              )                                                     \
        )
#else
#error "Unknown operating system in inst-power.h"
#endif


#define TRAMP_FRAME_SIZE_32 ALIGN_QUADWORD(STACKSKIP + GPRSAVE_32 + VECSAVE \
                                           + SPRSAVE_32 \
                                           + FUNCSAVE_32 + FUNCARGS_32 + LINKAREA_32)
#define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(STACKSKIP + GPRSAVE_64 + VECSAVE \
                                           + SPRSAVE_64 \
                                           + FUNCSAVE_64 + FUNCARGS_64 + LINKAREA_64)
#define PDYN_RESERVED_32 (LINKAREA_32 + FUNCARGS_32 + FUNCSAVE_32)
#define PDYN_RESERVED_64 (LINKAREA_64 + FUNCARGS_64 + FUNCSAVE_64)

#define TRAMP_SPR_OFFSET_32 (PDYN_RESERVED_32)
#define STK_LR       (              0)
#define STK_CR_32    (STK_LR      + 4)
#define STK_CTR_32   (STK_CR_32   + 4)
#define STK_XER_32   (STK_CTR_32  + 4)
#define STK_FP_CR_32 (STK_XER_32  + 4)
#define STK_SPR0_32  (STK_FP_CR_32+ 8)

#define TRAMP_SPR_OFFSET_64 (PDYN_RESERVED_64)
#define STK_CR_64    (STK_LR      + 8)
#define STK_CTR_64   (STK_CR_64   + 8)
#define STK_XER_64   (STK_CTR_64  + 8)
#define STK_FP_CR_64 (STK_XER_64  + 8)
#define STK_SPR0_64  (STK_FP_CR_64+ 8)

#define TRAMP_SPR_OFFSET(x) (((x) == 8) ? TRAMP_SPR_OFFSET_64 : TRAMP_SPR_OFFSET_32)

#define TRAMP_FPR_OFFSET_32 (TRAMP_SPR_OFFSET_32 + SPRSAVE_32)
#define TRAMP_FPR_OFFSET_64 (TRAMP_SPR_OFFSET_64 + SPRSAVE_64)
#define TRAMP_FPR_OFFSET(x) (((x) == 8) ? TRAMP_FPR_OFFSET_64 : TRAMP_FPR_OFFSET_32)

#define TRAMP_GPR_OFFSET_32 (TRAMP_FPR_OFFSET_32 + VECSAVE)
#define TRAMP_GPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + VECSAVE)
#define TRAMP_GPR_OFFSET(x) (((x) == 8) ? TRAMP_GPR_OFFSET_64 : TRAMP_GPR_OFFSET_32)

#define FUNC_CALL_SAVE_32 (LINKAREA_32 + FUNCARGS_32)
#define FUNC_CALL_SAVE_64 (LINKAREA_64 + FUNCARGS_64)
#define FUNC_CALL_SAVE(x) (((x) == 8) ? FUNC_CALL_SAVE_64 : FUNC_CALL_SAVE_32)

class codeGen;

void saveSPR(codeGen &gen,
             Register scratchReg,
             int sprnum,
             int stkOffset);
void restoreSPR(codeGen &gen,
                Register scratchReg,
                int sprnum,
                int stkOffset);
void saveLR(codeGen &gen,
            Register scratchReg,
            int stkOffset);
void restoreLR(codeGen &gen,
               Register scratchReg,
               int stkOffset);
void setBRL(codeGen &gen,
            Register scratchReg,
            long val,
            unsigned ti);
void saveCR(codeGen &gen,
            Register scratchReg,
            int stkOffset);
void restoreCR(codeGen &gen,
               Register scratchReg,
               int stkOffset);
void saveFPSCR(codeGen &gen,
               Register scratchReg,
               int stkOffset);
void restoreFPSCR(codeGen &gen,
                  Register scratchReg,
                  int stkOffset);
void saveRegister(codeGen &gen,
                  Register reg,
                  int save_off);
void restoreRegister(codeGen &gen,
                     Register source,
                     Register dest,
                     int save_off);
void restoreRegister(codeGen &gen,
                     Register reg,
                     int save_off);
void saveFPRegister(codeGen &gen,
                    Register reg,
                    int save_off);
void restoreFPRegister(codeGen &gen,
                       Register source,
                       Register dest,
                       int save_off);
void restoreFPRegister(codeGen &gen,
                       Register reg,
                       int save_off);
void pushStack(codeGen &gen);
void popStack(codeGen &gen);
unsigned saveGPRegisters(codeGen &gen, 
                         registerSpace *theRegSpace,
                         int save_off, int numReqGPRs=-1);
unsigned restoreGPRegisters(codeGen &gen, 
                            registerSpace *theRegSpace,
                            int save_off);
unsigned saveFPRegisters(codeGen &gen, 
                         registerSpace *theRegSpace,
                         int save_off);
unsigned restoreFPRegisters(codeGen &gen,
                            registerSpace *theRegSpace,
                            int save_off);
unsigned saveSPRegisters(codeGen &gen, registerSpace *,
                         int save_off, int force_save);
unsigned restoreSPRegisters(codeGen &gen, registerSpace *,
                            int save_off, int force_save);

#endif
