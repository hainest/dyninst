from dyninst import registers as dynregs
from zydis import registers as zyregs


class registers:
    """
    64-bit x86 registers

    Attributes:
        _zydis (dynregs.reg_list): registers from Zydis

            The general-purpose, floating-point (vector and scalar), and control registers.
            They have Zydis names with the form ZYDIS_REGISTER_<NAME>.

        dyninst (list[str]): register name from Dyninst

            Just the register names from Dyninst.

        all (list[str]): the set of registers to be imported into Dyninst

        aliases (list[str]): Registers with different names for the same physical register

            x86 has no registers aliased by name. These are _not_ the subregister aliases
            like eax as the lower 32-bits of rax.
    """

    def __init__(self, zydis_dir: str, dyn_dir: str):
        """
        Attributes
            zydis_dir (str): Zydis source directory
            dyn_dir (str): Dyninst source directory
        """
        self.dyninst_prefix = "x86_64"
        self.aliases = []
        self.dyninst = dynregs.read_registers(
            dyn_dir, self.dyninst_prefix, include_pseudo=True
        )
        _zydis = zyregs.read_registers(zydis_dir)
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

_lengths = {
    "FULL": {"desc": "64 bits", "sortid": -1},
    "BIT": {"desc": "1-bit EFLAGS", "sortid": 0},
    "L_REG": {"desc": "8-bit, first byte", "sortid": 1},
    "H_REG": {"desc": "8-bit, second byte", "sortid": 2},
    "W_REG": {"desc": "16-bit, first word", "sortid": 3},
    "D_REG": {"desc": "32 bit, first double word", "sortid": 4},
    "FPDBL": {"desc": "80-bit x87 FPU", "sortid": 5},
    "MMS": {"desc": "64-bit MMX and 3DNow!", "sortid": 6},
    "XMMS": {"desc": "128-bit SSE, FC16, XOP, AVX, and FMA3/4", "sortid": 7},
    "YMMS": {"desc": "256-bit SSE, AVX2, FMA3/4", "sortid": 8},
    "ZMMS": {"desc": "512-bit AVX-512/AVX10", "sortid": 9},
    "KMSKS": {"desc": "64-bit mask from AVX-512/AVX10", "sortid": 10},
}

_categories= {
    "GPR": {"desc": "General-Purpose Registers", "sortid": -1},
    "SEG": {"desc": "Segment Registers", "sortid": 0},
    "FLAG": {"desc": "RFLAGS Register", "sortid": 1},
    "MISC": {"desc": "Internal ProcControlAPI Register", "sortid": 2},
    "CTL": {"desc": "Control Registers CR0-CR7", "sortid": 3},
    "DBG": {"desc": "Debug Registers DR0-DR7", "sortid": 4},
    "TST": {"desc": "Internal InstructionAPI Registers", "sortid": 5},
    "X87": {"desc": "x87 FPU Registers", "sortid": 6},
    "MMX": {"desc": "MM0-MM7 Registers", "sortid": 7},
    "XMM": {"desc": "XMM0-XMM15 Registers from SSE (XMM0-XMM31 for AVX-512)", "sortid": 8},
    "YMM": {"desc": "YMM0-YMM15 Registers from AVX2/FMA (YMM0-YMM31 for AVX-512)", "sortid": 9},
    "ZMM": {"desc": "ZMM0-ZMM31 Registers from AVX-512", "sortid": 10},
    "KMASK": {"desc": "K0-K7 opmask Registers from AVX-512", "sortid": 11},
    "FPCTL": {"desc": "control/status Registers from x87, SSE, and AVX", "sortid": 12},
}
