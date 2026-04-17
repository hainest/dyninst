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

/*
 * emit-x86.h - x86 & AMD64 code generators
 * $Id: emit-x86.h,v 1.32 2008/09/11 20:14:14 mlam Exp $
 */

#ifndef _EMIT_X86_H
#define _EMIT_X86_H

#include <assert.h>
#include <vector>
#include "codegen/RegControl.h"
#include "common/src/headers.h"
#include "common/src/arch-x86.h"
#include "dyninstAPI/src/instPoint.h"
#include "baseTramp.h"
#include "function_cache.h"

#include "dyninstAPI/src/emitter.h"
#include "codegen/emitters/x86/Emitterx86.h"
#include "codegen/emitters/x86/IA32/EmitterIA32.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Dyn.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Stat.h"
#include "codegen/emitters/x86/AMD64/EmitterAMD64.h"
#include "codegen/emitters/x86/AMD64/EmitterAMD64Dyn.h"
#include "codegen/emitters/x86/AMD64/EmitterAMD64Stat.h"

class codeGen;
class registerSpace;

class registerSlot;

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;

// Emitter moved to emitter.h - useful on other platforms as well

#if defined(DYNINST_CODEGEN_ARCH_X86_64)


#endif

#endif
