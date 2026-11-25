#
# See the dyninst/COPYRIGHT file for copyright information.
#
# We provide the Paradyn Tools (below described as "Paradyn")
# on an AS IS basis, and do not warrant its validity or performance.
# We reserve the right to update, modify, or discontinue this
# software at any time.  We shall have no obligation to supply such
# updates or modifications or any other form of support to you.
#
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

import re


def _read_capstone_mnemonics(file: str) -> tuple[list[str], list[str]]:
    mnemonics = []
    with open(file, "r") as f:
        for line in f:
            line = re.sub(r"\s", "", line).lower()
            if not "riscv_ins_" in line:
                continue

            # Format: 'RISCV_INS_<MNEMONIC>,'
            mnemonic = line.replace("riscv_ins_", "").replace("aqrl", "aq_rl")
            if mnemonic == "ending":
                continue
            
            # Format: 'RISCV_INS_ALIAS_NOP, // Real instr.: RISCV_ADDI'
            if mnemonic.startswith("alias_"):
                (mnemonic, comment) = mnemonic.replace("alias_","").split(",")
                
            mnemonics.append(mnemonic.replace(",", ""))

    return sorted(mnemonics)


def _read_dyninst_mnemonics(file: str, dyninst_prefix: str):
    mnemonics = []
    with open(file, "r") as f:
        for line in f:
            entry = line.split(",", 1)[0]
            if "=" in entry:
                name, alias = [e.strip() for e in entry.split("=")]
                for n in [name, alias]:
                    if n.startswith(dyninst_prefix):
                        n = n[len(dyninst_prefix) + 1 :]
                    if n != "INVALID":
                        mnemonics.append(n)
            else:
                e = entry.strip()
                if e.startswith(dyninst_prefix):
                    e = e[len(dyninst_prefix) + 1 :]
                if e != "INVALID":
                    mnemonics.append(e)

    return sorted(mnemonics)


class mnemonics:
    def __init__(self, cap_dir: str, dyn_dir: str):
        self.dyninst_prefix = "riscv64_op"
        self.missing = []
        self.pseudo = []
        self.external = _read_capstone_mnemonics(cap_dir + "/include/capstone/riscv.h")
        self.dyninst = _read_dyninst_mnemonics(
            dyn_dir + "/common/h/mnemonics/riscv64_entryIDs.h", self.dyninst_prefix
        )
        self.aliases = []
