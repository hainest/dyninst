"""
Import PowerPC registers from Capstone
"""

from .dwarf import *
import dyninst

class registers:
    def __init__(self, cap_dir: str, dyn_dir: str):
        self.dyninst_prefix = "ppc64"
