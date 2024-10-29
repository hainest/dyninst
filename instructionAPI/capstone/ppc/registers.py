def _read_capstone_registers(file:str):
  
  def _find_reg_defs(fd):
    # Capstone register enumerations start with a '*_INVALID' entry
    for line in fd:
      if "PPC_REG_INVALID" in line:
        return True
    return False

  registers = []
  with open(file, "r") as f:
    if not _find_reg_defs(f):
      raise Exception("Unable to find register definitions in '{0:s}'".format(file)) 

    marker = "PPC_REG_"
    for line in f:
      if marker in line:
        # Format: PPC_REG_NAME = NUMBER,
        line = line.strip().replace(",", "")
        name = line[len(marker):line.find(' ')].lower()
        if name == "ending":
          break
        registers.append(name)

  return registers

def _read_dyninst_registers(file:str):
  regs = []
  
  with open(file, "r") as f:
    for line in f:
      
      # Don't include the internal pseudo-registers
      if "pseudo-registers" in line.lower():
        break
      
      if not "DEF_REGISTER(" in line:
        continue

      if "DEF_REGISTER_ALIAS" in line:
        # Format is DEF_REGISTER_ALIAS( X, Y, "ppc64")
        name, _ = line[line.index("DEF_REGISTER_ALIAS(") + 19:].split(",", 1)
        name = name.strip()
      else:
        # Format is DEF_REGISTER( x0, 0 | FULL | GPR | Arch_ppc64, "ppc64")
        name, _ = line[line.index("DEF_REGISTER(") + 10:].split(",", 1)
        name = name.strip()

      regs.append(name.lower())

  return sorted(regs)

_capstone_ignore = [
  # Internal Capstone/LLVM details
    "BP", "BP8",  # base pointer
    "FP", "FP8",  # frame pointer
    "CARRY",      # bit 0 of the XER register
    "RM",         # rounding mode, bits 30 and 31 of FPSCR
]
_dyninst_special = [
  "CR",     # the whole condition register; cr0..7
  "PC",     # Power has no Program Counter
  "FPSCR",  # Floating-Point Status and Control Register; implicitly used by instructions like mtfsb0; part of ABI
]
_dyninst_dead = [
  # Not directly addressable. Implicitly read/written via m{f,t}spr instructions (ISA v2.01)
  "ppr",      # 3.2.3 Program Priority Register
  "ppr32",    # 3.2.3 Program Priority Register (upper 32 bits of PPR)
]
_capstone_to_dyninst = {
  # F0..31     -> fpr0..31
  # S0..31     -> fsr0..31
  # VSL0..31   -> vsr0..31
  # VSX32..63  -> vsr32..63
  # CR<n>{eq,gt,lt,un} -> cr<n>{l,g,e} (un isn't in Dyninst, it's unordered FP; Section 2.3.1)
}
_category_details = {
  "SPR4": {"size": 32,   "desc": "32-bit Special-purpose Registers"},
  "GPR":  {"size": 64,   "desc": "General-purpose Registers"},
  "SPR":  {"size": 64,   "desc": "64-bit Special-purpose Registers"},
  "SPE":  {"size": 64,   "desc": "Signal Processing Engine extension"},
  "GP8":  {"size": 128,  "desc": "Consecutive even-odd paired GPRs (G8)"},
  "QPX":  {"size": 256,  "desc": "Quad-Vector Floating-Point Registers"},
  "VSX":  {"size": 512,  "desc": "Vector-Scalar extension"},
  "MMA":  {"size": 512,  "desc": "Math Matrix Accelerator (Power10)"},
  "DMR":  {"size": 1024, "desc": "Dense Math Registers (Power10)"},
}
_alias_by_prefix = {
  "GP8":  "R",
  "X":    "R",
  "VF":   "VSX",    # vf0..31 are old names for vs32..63
  "V":    "VSX",    # same as VF
}
_alias_by_name = {
  "ZERO":  "R0",  # r0 when treated as the constant 0
  "ZERO8": "R0",
  "LR8": "LR",    # link register
  "CTR8": "CTR",  # count register
}
_category_by_prefix = {
  "R":   "GPR",   # r0..31
  "F":   "FPR",   # f0..7
  "CR":  "SPR4",  # cr0..7
  "LR":  "SPR",   # link register
  "CTR": "SPR",   # count register
  "XER": "SPR",   # fixed-point exception register
  "G8p": "GP8",
  
  # Quad-Vector Floating-Point
  "QF": "QPX",    # qf0..32

  # Signal Processing extension
  "S":        "SPE",  # s0..31
  "SPEFSCR":  "SPE",  # Floating-Point Status and Control Register
  
  # Vector-Scalar extension
  "VSL": "VSX",     # vs0..31
  "VSX": "VSX",     # vs32..63
  
  # Power10 Dense Math Registers
  #   There are no official docs on these yet. I've just directly
  #   copied the values from the LLVM tablegen and gcc patches
  #   (https://gcc.gnu.org/pipermail/gcc-patches/2022-November/605581.html)
  "DMR":     "DMR",  # DMR0..7
  "WACC":    "DMR",  # WACC0..3
  "WACC_HI": "DMR",  # WACC_HI0..3
  "DMRROWp": "DMR",  # DMRROWp0..31
  "DMRROW":  "DMR",  # DMRROW0..63
  "DMRp":    "DMR",  # DMRp0..3
  
  # Power10 Math Matrix Accelerator
  #   VSR paired registers VSRp<n> are two 128-bit VSX registers
  "VSRp": "MMA",  # VSRp0..31
  "ACC":  "MMA",  # ACC0..7, 512-bit VSX accumulators
  "UACC": "MMA",  # unprimed ACC0..7
}


class registers:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix="ppc64"
    self.capstone = _read_capstone_registers(cap_dir + "/include/capstone/ppc.h")
    self.dyninst = _read_dyninst_registers(dyn_dir + "/common/h/registers/ppc64_regs.h")
    self.aliases = None
    self.all = None

  @staticmethod
  def export_lengths(f):
    pass

  @staticmethod
  def export_categories(f):
    pass

  @staticmethod
  def export_dwarf(f):
    pass

def _capstone_to_dyninst():
  """ Map Capstone register enum values to Dyninst MachRegister objects """
  pass

def _process_regs(capstone, capstone_sysregs, spec_sysregs):
  pass
