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

#ifndef DYNINST_DYNINSTAPI_PATCHING_PATCH_EDGE_H
#define DYNINST_DYNINSTAPI_PATCHING_PATCH_EDGE_H

#include "CFG.h"
#include "PatchCFG.h"
#include "addressSpace.h"
#include "patching/block.h"
#include "mapped_object.h"

namespace Dyninst { namespace DyninstAPI {

  class patch_edge : public PatchAPI::PatchEdge {
  public:
    patch_edge(ParseAPI::Edge *edge, block_instance *source, block_instance *target);

    patch_edge(const patch_edge *parent, mapped_object *child);

    ~patch_edge() = default;

    block_instance *src() const;

    block_instance *trg() const;

    AddressSpace *proc();
  };

}}

#endif
