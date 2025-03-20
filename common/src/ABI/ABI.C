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

#include "abi.h"
#include "ABI/x86.h"
#include "ABI/x86_64.h"
#include "ABI/ppc64.h"
#include "ABI/aarch64.h"
#include "registers/registerSet.h"
#include "ABI/architecture.h"

#include <memory>

namespace Dyninst { namespace abi {

  struct abi_impl final {
    Dyninst::abi::architecture machine;
    Dyninst::Architecture arch;

    abi_impl(Dyninst::Architecture a) : arch(a) {
      switch(a) {
        case Dyninst::Arch_x86:
          machine = abi::make_x86();
          break;
        case Dyninst::Arch_x86_64:
          machine = abi::make_x86_64();
          break;
        case Dyninst::Arch_ppc64:
          machine = abi::make_ppc64();
          break;
        case Dyninst::Arch_aarch64:
          machine = abi::make_aarch64();
          break;
        case Dyninst::Arch_ppc32:
        case Dyninst::Arch_aarch32:
        case Dyninst::Arch_amdgpu_gfx908:
        case Dyninst::Arch_amdgpu_gfx90a:
        case Dyninst::Arch_amdgpu_gfx940:
        case Dyninst::Arch_cuda:
        case Dyninst::Arch_intelGen9:
        case Dyninst::Arch_none:
          break;
      }
    }
  };

}}

Dyninst::ABI::ABI(Dyninst::Architecture a) : impl(new Dyninst::abi::abi_impl(a)) {}

Dyninst::registerSet const& Dyninst::ABI::getFunctionParams() const {
  return impl->machine.function.params;
}
Dyninst::registerSet const& Dyninst::ABI::getFunctionReturns() const {
  return impl->machine.function.returns;
}
Dyninst::registerSet const& Dyninst::ABI::getFunctionPreserved() const {
  return impl->machine.function.preserved;
}
Dyninst::registerSet const& Dyninst::ABI::getFunctionGlobals() const {
  return impl->machine.function.globals;
}
Dyninst::registerSet const& Dyninst::ABI::getSyscallParams() const {
  return impl->machine.syscall.params;
}
Dyninst::registerSet const& Dyninst::ABI::getSyscallReturns() const {
  return impl->machine.syscall.returns;
}
Dyninst::registerSet const& Dyninst::ABI::getSyscallPreserved() const {
  return impl->machine.syscall.preserved;
}
Dyninst::registerSet const& Dyninst::ABI::getSyscallGlobals() const {
  return impl->machine.syscall.globals;
}
Dyninst::Architecture Dyninst::ABI::getArchitecture() const {
  return impl->arch;
}