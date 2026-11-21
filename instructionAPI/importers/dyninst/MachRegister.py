"""
Dyninst MachRegister
"""

from dataclasses import dataclass


@dataclass
class MachRegister:
    """
    A Dyninst MachRegister

    Attributes:
        name (str): Register name
        id (str): Base IDs for aliased regs (e.g., BASEA for eax/rax)
        alias (str): Register length (FULL, L_REG, etc.)
        category (str): Register category (GPR, SPR, etc.)
        name_alias (str): Third-party name alias
    """

    name: str
    id: str
    alias: str
    category: str
    arch: str
    name_alias: str


def from_alias(base_name: str, alias_name: str, arch: str) -> MachRegister:
    """
    Create a MachRegister from a DEF_REGISTER_ALIAS declaration
    """
    return MachRegister(base_name, "", "", "", arch, alias_name)


def from_definition(*args, arch: str) -> MachRegister:
    """
    Create a MachRegister from a DEF_REGISTER declaration
    """
    if len(args) == 3:
        return MachRegister(*args, "", arch, "")

    return MachRegister(*args, arch, "")
