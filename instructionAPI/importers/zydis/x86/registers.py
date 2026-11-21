from dyninst import registers as dynregs
from zydis import registers as zyregs


class registers:
    """
    32-bit x86 registers

    Attributes:
        _zydis (dynregs.reg_list): registers from Zydis

            The general-purpose, floating-point (vector and scalar), and control registers.
            They have Zydis names with the form ZYDIS_REGISTER_<NAME>.

        _dyninst (dynregs.reg_list): registers declared in Dyninst

            The complete information (as dyninst.registers.Register) for registers from Dyninst.

        dyninst (list[str]): register name from Dyninst

            Just the register names from Dyninst.

        all (list[str]): the set of registers to be imported into Dyninst

        aliases (list[str]): Registers with different names for the same physical register

            x86 has no registers aliased by name. These are _not_ the subregisters aliases
            like ax  as the lower 16-bits of rax.
    """

    def __init__(self, zydis_dir: str, dyn_dir: str):
        """
        Attributes
            zydis_dir (str): Zydis source directory
            dyn_dir (str): Dyninst source directory
        """
        self.dyninst_prefix = "x86"
        self.aliases = []
        _dyninst = dynregs.read_registers(dyn_dir, dyninst_prefix, include_pseudo=True)
        _zydis = zyregs.read_registers(zydis_dir)
        self.dyninst = [r.name for r in _dyninst]
        self.all = zyregs.merge(_zydis, _dyninst)

    @staticmethod
    def export_lengths(f):
        pass
        # max_len = max([len(l) for l in _lengths])
        # id = 0
        # decl = "const int32_t %-{0:d}s = 0x0000%02X00;  // %s\n".format(max_len)

    @staticmethod
    def export_categories(f):
        pass
        # max_len = max([len(d) for d in _basic_category_descriptions])
        # id = 0
        # decl = "const int32_t %-{0:d}s = 0x00%02X0000;  // %s\n".format(max_len)

    @staticmethod
    def export_dwarf(f):
        pass
