"""
Helper function for reading Dyninst registers.
"""

import dyninst.MachRegister as MachRegister
import re


def load(src_dir: str, arch: str) -> list[MachRegister]:
    """
    Parse all registers from Dyninst for `arch`

    Parameters:
        src_dir (str): Dyninst source directory
        arch (str): The architecture (e.g., x86_64)
        include_pseudo (bool): Include pseudo registers

    Returns:
        A list of the MachRegisters for `arch`.
    """

    regs: list[MachRegister] = []

    file = src_dir + f"/common/h/registers/{arch}_regs.def"
    with open(file, "r", encoding="utf-8") as f:
        for line in f:
            line = re.sub(r"\s+", "", line).lower()

            if not line.startswith("def_register"):
                continue

            if line.startswith("def_register_alias("):
                # Format is DEF_REGISTER_ALIAS(X, Y)
                istart = line.index("def_register_alias(") + 19
                iend = line.index(")")
                (name, alias) = line[istart:iend].split(",")
                regs.append(MachRegister.from_alias(name, alias, arch))
            else:
                # Format is DEF_REGISTER(<name>, <id> | <alias> | <category> | Arch_<ARCH>)
                istart = line.index("def_register(") + 13
                iend = line.index(")")
                args = line[istart:iend].split("|")
                regs.append(MachRegister.from_definition(*args, arch=arch))

    print(f"Found {len(regs)} Dyninst registers")

    return regs
