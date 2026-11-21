"""
Register types and helper functions for Zydis
"""

from dyninst import registers as dynregs


def read_registers(src_dir: str) -> dynregs.reg_list:
    """
    Parameters:
        src_dir (str): Zydis source directory
    """

    regs: dynregs.reg_list = []

    aliases = {"ia32_kernel_gs_base": "gsbase"}

    file = src_dir + "/include/Zydis/Generated/EnumRegister.h"

    with open(file, "r", encoding="utf-8") as f:
        for line in f:
            if "ZYDIS_REGISTER" not in line:
                continue

            # This is effectively the last entry
            if "ZYDIS_REGISTER_MAX_VALUE" in line:
                break

            # Format is ZYDIS_REGISTER_<NAME>,
            line = line.replace(",", "").strip()
            name = line.replace("ZYDIS_REGISTER_", "").lower()

            # ZYDIS_REGISTER_NONE needs to be mapped to Dyninst::InvalidReg
            # in the decoder, but we don't make an explicit imported register for it.
            if name == "none":
                continue

            alias = ""
            if name in aliases:
                alias = aliases[name]

            regs.append(dynregs.Register(name, name_alias=alias))

    print(f"Found {len(regs)} Zydis registers")

    return regs
