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
 * emit-x86.C - x86 & AMD64 code generators
 * $Id: emit-x86.C,v 1.64 2008/09/11 20:14:14 mlam Exp $
 */

#include <assert.h>
#include <stdio.h>
#include "codegen/RegControl.h"
#include "common/src/bitmath.h"
#include "compiler_annotations.h"
#include "codegen/codegen.h"
#include "patching/function.h"
#include "Symbol.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"
#include "ASTs/ast.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "registerSpace/registerSpace.h"
#include "arch-regs-x86.h"
#include "dynproc/dynProcess.h"

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
// get_index...
#include "dynproc/dynThread.h"
#include "ABI.h"
#include "liveness.h"
#include "registerSpace/RegisterConversion.h"
#include "unaligned_memory_access.h"
#include "codegen/emitters/x86/generators.h"
#include "codegen/emitters/x86/AMD64/generators.h"

//
// 64-bit code generation helper functions
//


