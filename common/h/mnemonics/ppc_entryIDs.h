// Only in Dyninst
power_op_lfq,
power_op_lfqu,
power_op_lfqux,
power_op_lfqx,
power_op_lswx,
power_op_stfq,
power_op_stfqu,
power_op_stfqux,
power_op_stfqx,
power_op_stswx,
power_op_addg6s,
power_op_cbcdtd,
power_op_cdtbcd,
power_op_cmp,
power_op_cmpi,
power_op_cmpl,
power_op_cmpli,
power_op_fmrgew,
power_op_fmrgow,
power_op_eciwx,
power_op_ecowx,
power_op_dadd,
power_op_daddq,
power_op_dcffix,
power_op_dcffixq,
power_op_dcffixqq,
power_op_dcmpo,
power_op_dcmpoq,
power_op_dcmpu,
power_op_dcmpuq,
power_op_dctdp,
power_op_dctfix,
power_op_dctfixq,
power_op_dctqpq,
power_op_ddedpd,
power_op_ddedpdq,
power_op_ddiv,
power_op_ddivq,
power_op_denbcd,
power_op_denbcdq,
power_op_diex,
power_op_diexq,
power_op_dmul,
power_op_dmulq,
power_op_dqua,
power_op_dquai,
power_op_dquaiq,
power_op_dquaq,
power_op_drdpq,
power_op_drintn,
power_op_drintnq,
power_op_drintx,
power_op_drintxq,
power_op_drrnd,
power_op_drrndq,
power_op_drsp,
power_op_dscli,
power_op_dscliq,
power_op_dscri,
power_op_dscriq,
power_op_dsub,
power_op_dsubq,
power_op_dtstdc,
power_op_dtstdcq,
power_op_dtstdg,
power_op_dtstdgq,
power_op_dtstex,
power_op_dtstexq,
power_op_dtstsf,
power_op_dtstsfi,
power_op_dtstsfiq,
power_op_dtstsfq,
power_op_dxex,
power_op_dxexq,
power_op_lfdpx,
power_op_mcrxr,
power_op_msgclr,
power_op_msgclrp,
power_op_msgsnd,
power_op_msgsndp,
power_op_prtyd,
power_op_prtyw,
power_op_slbiag,
power_op_stfdpx,
// From Capstone
power_op_INVALID, /* pseudo mnemonic */
power_op_extended, /* pseudo mnemonic */
power_op_add,
power_op_addc,
power_op_addco,
power_op_adde,
power_op_addeo,
power_op_addex,
power_op_addi,
power_op_addic,
power_op_addic_rc = power_op_addic,
power_op_addis,
power_op_addme,
power_op_addmeo,
power_op_addo,
power_op_addpcis,
power_op_addze,
power_op_addzeo,
power_op_and,
power_op_andc,
power_op_andi,
power_op_andi_rc = power_op_andi,
power_op_andis,
power_op_andis_rc = power_op_andis,
power_op_attn,
power_op_b,
power_op_ba,
power_op_bc,
power_op_bca,
power_op_bcctr,
power_op_bcctrl,
power_op_bcdadd,
power_op_bcdcfn,
power_op_bcdcfsq,
power_op_bcdcfz,
power_op_bcdcpsgn,
power_op_bcdctn,
power_op_bcdctsq,
power_op_bcdctz,
power_op_bcds,
power_op_bcdsetsgn,
power_op_bcdsr,
power_op_bcdsub,
power_op_bcdtrunc,
power_op_bcdus,
power_op_bcdutrunc,
power_op_bcl,
power_op_bcla,
power_op_bclr,
power_op_bclrl,
power_op_bctr,
power_op_bctrl,
power_op_bl,
power_op_bla,
power_op_blr,
power_op_blrl,
power_op_bpermd,
power_op_brd,
power_op_brh,
power_op_brinc,
power_op_brw,
power_op_cfuged,
power_op_clrbhrb,
power_op_clrlsldi,
power_op_clrlslwi,
power_op_clrrdi,
power_op_clrrwi,
power_op_cmpb,
power_op_cmpd,
power_op_cmpdi,
power_op_cmpeqb,
power_op_cmpld,
power_op_cmpldi,
power_op_cmplw,
power_op_cmplwi,
power_op_cmprb,
power_op_cmpw,
power_op_cmpwi,
power_op_cntlzd,
power_op_cntlzdm,
power_op_cntlzw,
power_op_cnttzd,
power_op_cnttzdm,
power_op_cnttzw,
power_op_copy,
power_op_cpabort,
power_op_cp_abort = power_op_cpabort,
power_op_crand,
power_op_crandc,
power_op_creqv,
power_op_crnand,
power_op_crnor,
power_op_cror,
power_op_crorc,
power_op_crxor,
power_op_darn,
power_op_dcba,
power_op_dcbf,
power_op_dcbfep,
power_op_dcbfl,
power_op_dcbflp,
power_op_dcbfps,
power_op_dcbi,
power_op_dcbst,
power_op_dcbstep,
power_op_dcbstps,
power_op_dcbt,
power_op_dcbtct,
power_op_dcbtds,
power_op_dcbtep,
power_op_dcbtst,
power_op_dcbtstct,
power_op_dcbtstds,
power_op_dcbtstep,
power_op_dcbtstt,
power_op_dcbtt,
power_op_dcbz,
power_op_dcbz_l,
power_op_dcbzep,
power_op_dcbzl,
power_op_dcbzlep,
power_op_dccci,
power_op_divd,
power_op_divde,
power_op_divdeo,
power_op_divdeu,
power_op_divdeuo,
power_op_divdo,
power_op_divdu,
power_op_divduo,
power_op_divw,
power_op_divwe,
power_op_divweo,
power_op_divweu,
power_op_divweuo,
power_op_divwo,
power_op_divwu,
power_op_divwuo,
power_op_dmmr,
power_op_dmsetdmrz,
power_op_dmxor,
power_op_dmxxextfdmr256,
power_op_dmxxextfdmr512,
power_op_dmxxinstfdmr256,
power_op_dmxxinstfdmr512,
power_op_dss,
power_op_dssall,
power_op_dst,
power_op_dstst,
power_op_dststt,
power_op_dstt,
power_op_efdabs,
power_op_efdadd,
power_op_efdcfs,
power_op_efdcfsf,
power_op_efdcfsi,
power_op_efdcfsid,
power_op_efdcfuf,
power_op_efdcfui,
power_op_efdcfuid,
power_op_efdcmpeq,
power_op_efdcmpgt,
power_op_efdcmplt,
power_op_efdctsf,
power_op_efdctsi,
power_op_efdctsidz,
power_op_efdctsiz,
power_op_efdctuf,
power_op_efdctui,
power_op_efdctuidz,
power_op_efdctuiz,
power_op_efddiv,
power_op_efdmul,
power_op_efdnabs,
power_op_efdneg,
power_op_efdsub,
power_op_efdtsteq,
power_op_efdtstgt,
power_op_efdtstlt,
power_op_efsabs,
power_op_efsadd,
power_op_efscfd,
power_op_efscfsf,
power_op_efscfsi,
power_op_efscfuf,
power_op_efscfui,
power_op_efscmpeq,
power_op_efscmpgt,
power_op_efscmplt,
power_op_efsctsf,
power_op_efsctsi,
power_op_efsctsiz,
power_op_efsctuf,
power_op_efsctui,
power_op_efsctuiz,
power_op_efsdiv,
power_op_efsmul,
power_op_efsnabs,
power_op_efsneg,
power_op_efssub,
power_op_efststeq,
power_op_efststgt,
power_op_efststlt,
power_op_eieio,
power_op_eqv,
power_op_evabs,
power_op_evaddiw,
power_op_evaddsmiaaw,
power_op_evaddssiaaw,
power_op_evaddumiaaw,
power_op_evaddusiaaw,
power_op_evaddw,
power_op_evand,
power_op_evandc,
power_op_evcmpeq,
power_op_evcmpgts,
power_op_evcmpgtu,
power_op_evcmplts,
power_op_evcmpltu,
power_op_evcntlsw,
power_op_evcntlzw,
power_op_evdivws,
power_op_evdivwu,
power_op_eveqv,
power_op_evextsb,
power_op_evextsh,
power_op_evfsabs,
power_op_fsabs = power_op_evfsabs,
power_op_evfsadd,
power_op_evfscfsf,
power_op_evfscfsi,
power_op_evfscfuf,
power_op_evfscfui,
power_op_evfscmpeq,
power_op_evfscmpgt,
power_op_fscmp = power_op_evfscmpgt,
power_op_evfscmplt,
power_op_evfsctsf,
power_op_evfsctsi,
power_op_evfsctsiz,
power_op_evfsctui,
power_op_evfsdiv,
power_op_evfsmul,
power_op_evfsnabs,
power_op_fsnabs = power_op_evfsnabs,
power_op_evfsneg,
power_op_fsneg = power_op_evfsneg,
power_op_evfssub,
power_op_evfststeq,
power_op_evfststgt,
power_op_evfststlt,
power_op_evldd,
power_op_evlddx,
power_op_evldh,
power_op_evldhx,
power_op_evldw,
power_op_evldwx,
power_op_evlhhesplat,
power_op_evlhhesplatx,
power_op_evlhhossplat,
power_op_evlhhossplatx,
power_op_evlhhousplat,
power_op_evlhhousplatx,
power_op_evlwhe,
power_op_evlwhex,
power_op_evlwhos,
power_op_evlwhosx,
power_op_evlwhou,
power_op_evlwhoux,
power_op_evlwhsplat,
power_op_evlwhsplatx,
power_op_evlwwsplat,
power_op_evlwwsplatx,
power_op_evmergehi,
power_op_evmergehilo,
power_op_evmergelo,
power_op_evmergelohi,
power_op_evmhegsmfaa,
power_op_evmhegsmfan,
power_op_evmhegsmiaa,
power_op_evmhegsmian,
power_op_evmhegumiaa,
power_op_evmhegumian,
power_op_evmhesmf,
power_op_evmhesmfa,
power_op_evmhesmfaaw,
power_op_evmhesmfanw,
power_op_evmhesmi,
power_op_evmhesmia,
power_op_evmhesmiaaw,
power_op_evmhesmianw,
power_op_evmhessf,
power_op_evmhessfa,
power_op_evmhessfaaw,
power_op_evmhessfanw,
power_op_evmhessiaaw,
power_op_evmhessianw,
power_op_evmheumi,
power_op_evmheumia,
power_op_evmheumiaaw,
power_op_evmheumianw,
power_op_evmheusiaaw,
power_op_evmheusianw,
power_op_evmhogsmfaa,
power_op_evmhogsmfan,
power_op_evmhogsmiaa,
power_op_evmhogsmian,
power_op_evmhogumiaa,
power_op_evmhogumian,
power_op_evmhosmf,
power_op_evmhosmfa,
power_op_evmhosmfaaw,
power_op_evmhosmfanw,
power_op_evmhosmi,
power_op_evmhosmia,
power_op_evmhosmiaaw,
power_op_evmhosmianw,
power_op_evmhossf,
power_op_evmhossfa,
power_op_evmhossfaaw,
power_op_evmhossfanw,
power_op_evmhossiaaw,
power_op_evmhossianw,
power_op_evmhoumi,
power_op_evmhoumia,
power_op_evmhoumiaaw,
power_op_evmhoumianw,
power_op_evmhousiaaw,
power_op_evmhousianw,
power_op_evmra,
power_op_evmwhsmf,
power_op_evmwhsmfa,
power_op_evmwhsmi,
power_op_evmwhsmia,
power_op_evmwhssf,
power_op_evmwhssfa,
power_op_evmwhumi,
power_op_evmwhumia,
power_op_evmwlsmiaaw,
power_op_evmwlsmianw,
power_op_evmwlssiaaw,
power_op_evmwlssianw,
power_op_evmwlumi,
power_op_evmwlumia,
power_op_evmwlumiaaw,
power_op_evmwlumianw,
power_op_evmwlusiaaw,
power_op_evmwlusianw,
power_op_evmwsmf,
power_op_evmwsmfa,
power_op_evmwsmfaa,
power_op_evmwsmfan,
power_op_evmwsmi,
power_op_evmwsmia,
power_op_evmwsmiaa,
power_op_evmwsmian,
power_op_evmwssf,
power_op_evmwssfa,
power_op_evmwssfaa,
power_op_evmwssfan,
power_op_evmwumi,
power_op_evmwumia,
power_op_evmwumiaa,
power_op_evmwumian,
power_op_evnand,
power_op_evneg,
power_op_evnor,
power_op_evor,
power_op_evorc,
power_op_evrlw,
power_op_evrlwi,
power_op_evrndw,
power_op_evsel,
power_op_evslw,
power_op_evslwi,
power_op_evsplatfi,
power_op_evsplati,
power_op_evsrwis,
power_op_evsrwiu,
power_op_evsrws,
power_op_evsrwu,
power_op_evstdd,
power_op_evstddx,
power_op_evstdh,
power_op_evstdhx,
power_op_evstdw,
power_op_evstdwx,
power_op_evstwhe,
power_op_evstwhex,
power_op_evstwho,
power_op_evstwhox,
power_op_evstwwe,
power_op_evstwwex,
power_op_evstwwo,
power_op_evstwwox,
power_op_evsubfsmiaaw,
power_op_evsubfssiaaw,
power_op_evsubfumiaaw,
power_op_evsubfusiaaw,
power_op_evsubfw,
power_op_evsubifw,
power_op_evxor,
power_op_extldi,
power_op_extlwi,
power_op_extrdi,
power_op_extrwi,
power_op_extsb,
power_op_extsh,
power_op_extsw,
power_op_extswsli,
power_op_extswsl = power_op_extswsli,
power_op_fabs,
power_op_fpabs = power_op_fabs,
power_op_fadd,
power_op_fpadd = power_op_fadd,
power_op_fadds,
power_op_fcfid,
power_op_fcfids,
power_op_fcfidu,
power_op_fcfidus,
power_op_fcmpo,
power_op_fcmpu,
power_op_fcpsgn,
power_op_fctid,
power_op_fctidu,
power_op_fctiduz,
power_op_fctidz,
power_op_fctiw,
power_op_fpctiw = power_op_fctiw,
power_op_fctiwu,
power_op_fctiwuz,
power_op_fctiwz,
power_op_fpctiwz = power_op_fctiwz,
power_op_fdiv,
power_op_fdivs,
power_op_fmadd,
power_op_fpmadd = power_op_fmadd,
power_op_fmadds,
power_op_fmr,
power_op_fpmr = power_op_fmr,
power_op_fmsub,
power_op_fpmsub = power_op_fmsub,
power_op_fmsubs,
power_op_fmul,
power_op_fpmul = power_op_fmul,
power_op_fmuls,
power_op_fnabs,
power_op_fpnabs = power_op_fnabs,
power_op_fneg,
power_op_fpneg = power_op_fneg,
power_op_fnmadd,
power_op_fpnmadd = power_op_fnmadd,
power_op_fnmadds,
power_op_fnmsub,
power_op_fpnmsub = power_op_fnmsub,
power_op_fnmsubs,
power_op_fre,
power_op_fpre = power_op_fre,
power_op_fres,
power_op_frim,
power_op_frin,
power_op_frip,
power_op_friz,
power_op_frsp,
power_op_fprsp = power_op_frsp,
power_op_frsqrte,
power_op_fprsqrte = power_op_frsqrte,
power_op_frsqrtes,
power_op_fsel,
power_op_fpsel = power_op_fsel,
power_op_fsqrt,
power_op_fsqrts,
power_op_fsub,
power_op_fpsub = power_op_fsub,
power_op_fsubs,
power_op_ftdiv,
power_op_ftsqrt,
power_op_hashchk,
power_op_hashchkp,
power_op_hashst,
power_op_hashstp,
power_op_hrfid,
power_op_icbi,
power_op_icbiep,
power_op_icblc,
power_op_icblq,
power_op_icbt,
power_op_icbtls,
power_op_iccci,
power_op_inslwi,
power_op_insrdi,
power_op_insrwi,
power_op_invalid,
power_op_isel,
power_op_isync,
power_op_la,
power_op_lbarx,
power_op_lbepx,
power_op_lbz,
power_op_lbzcix,
power_op_lbzu,
power_op_lbzux,
power_op_lbzx,
power_op_ld,
power_op_ldarx,
power_op_ldat,
power_op_ldbrx,
power_op_ldcix,
power_op_ldu,
power_op_ldux,
power_op_ldx,
power_op_lfd,
power_op_lfdepx,
power_op_lfdu,
power_op_lfdux,
power_op_lfdx,
power_op_lfiwax,
power_op_lfiwzx,
power_op_lfs,
power_op_lfsu,
power_op_lfsux,
power_op_lfsx,
power_op_lha,
power_op_lharx,
power_op_lhau,
power_op_lhaux,
power_op_lhax,
power_op_lhbrx,
power_op_lhepx,
power_op_lhz,
power_op_lhzcix,
power_op_lhzu,
power_op_lhzux,
power_op_lhzx,
power_op_lmw,
power_op_lq,
power_op_lqarx,
power_op_lswi,
power_op_lvebx,
power_op_lvehx,
power_op_lvewx,
power_op_lvsl,
power_op_lvsr,
power_op_lvx,
power_op_lvxl,
power_op_lwa,
power_op_lwarx,
power_op_lwat,
power_op_lwaux,
power_op_lwax,
power_op_lwbrx,
power_op_lwepx,
power_op_lwz,
power_op_lwzcix,
power_op_lwzu,
power_op_lwzux,
power_op_lwzx,
power_op_lxsd,
power_op_lxsdx,
power_op_lxsibzx,
power_op_lxsihzx,
power_op_lxsiwax,
power_op_lxsiwzx,
power_op_lxssp,
power_op_lxsspx,
power_op_lxv,
power_op_lxvb16x,
power_op_lxvd2x,
power_op_lxvdsx,
power_op_lxvh8x,
power_op_lxvkq,
power_op_lxvl,
power_op_lxvll,
power_op_lxvp,
power_op_lxvprl,
power_op_lxvprll,
power_op_lxvpx,
power_op_lxvrbx,
power_op_lxvrdx,
power_op_lxvrhx,
power_op_lxvrl,
power_op_lxvrll,
power_op_lxvrwx,
power_op_lxvw4x,
power_op_lxvwsx,
power_op_lxvx,
power_op_maddhd,
power_op_maddhdu,
power_op_maddld,
power_op_mbar,
power_op_mcrf,
power_op_mcrfs,
power_op_mcrxrx,
power_op_mfbhrbe,
power_op_mfcr,
power_op_mfctr,
power_op_mfdcr,
power_op_mffs,
power_op_mffscdrn,
power_op_mffscdrni,
power_op_mffsce,
power_op_mffscrn,
power_op_mffscrni,
power_op_mffsl,
power_op_mflr,
power_op_mfmsr,
power_op_mfocrf,
power_op_mfpmr,
power_op_mfspr,
power_op_mfsr,
power_op_mfsrin,
power_op_mftb,
power_op_mfvscr,
power_op_mfvsrd,
power_op_mfvsrld,
power_op_mfvsrwz,
power_op_modsd,
power_op_modsw,
power_op_modud,
power_op_moduw,
power_op_msgsync,
power_op_mtcrf,
power_op_mtctr,
power_op_mtdcr,
power_op_mtfsb0,
power_op_mtfsb1,
power_op_mtfsf,
power_op_mtfsfi,
power_op_mtlr,
power_op_mtmsr,
power_op_mtmsrd,
power_op_mtocrf,
power_op_mtpmr,
power_op_mtspr,
power_op_mtsr,
power_op_mtsrin,
power_op_mtvscr,
power_op_mtvsrbm,
power_op_mtvsrbmi,
power_op_mtvsrd,
power_op_mtvsrdd,
power_op_mtvsrdm,
power_op_mtvsrhm,
power_op_mtvsrqm,
power_op_mtvsrwa,
power_op_mtvsrwm,
power_op_mtvsrws,
power_op_mtvsrwz,
power_op_mulhd,
power_op_mulhdu,
power_op_mulhw,
power_op_mulhwu,
power_op_mulld,
power_op_mulldo,
power_op_mulli,
power_op_mullw,
power_op_mullwo,
power_op_nand,
power_op_nap,
power_op_neg,
power_op_nego,
power_op_nop,
power_op_nor,
power_op_or,
power_op_orc,
power_op_ori,
power_op_oris,
power_op_paddi,
power_op_paste,
power_op_pdepd,
power_op_pextd,
power_op_plbz,
power_op_pld,
power_op_plfd,
power_op_plfs,
power_op_plha,
power_op_plhz,
power_op_pli,
power_op_plwa,
power_op_plwz,
power_op_plxsd,
power_op_plxssp,
power_op_plxv,
power_op_plxvp,
power_op_pmxvbf16ger2,
power_op_pmxvbf16ger2nn,
power_op_pmxvbf16ger2np,
power_op_pmxvbf16ger2pn,
power_op_pmxvbf16ger2pp,
power_op_pmxvf16ger2,
power_op_pmxvf16ger2nn,
power_op_pmxvf16ger2np,
power_op_pmxvf16ger2pn,
power_op_pmxvf16ger2pp,
power_op_pmxvf32ger,
power_op_pmxvf32gernn,
power_op_pmxvf32gernp,
power_op_pmxvf32gerpn,
power_op_pmxvf32gerpp,
power_op_pmxvf64ger,
power_op_pmxvf64gernn,
power_op_pmxvf64gernp,
power_op_pmxvf64gerpn,
power_op_pmxvf64gerpp,
power_op_pmxvi16ger2,
power_op_pmxvi16ger2pp,
power_op_pmxvi16ger2s,
power_op_pmxvi16ger2spp,
power_op_pmxvi4ger8,
power_op_pmxvi4ger8pp,
power_op_pmxvi8ger4,
power_op_pmxvi8ger4pp,
power_op_pmxvi8ger4spp,
power_op_popcntb,
power_op_popcntd,
power_op_popcntw,
power_op_ps_abs,
power_op_ps_add,
power_op_ps_cmpo0,
power_op_ps_cmpo1,
power_op_ps_cmpu0,
power_op_ps_cmpu1,
power_op_ps_div,
power_op_ps_madd,
power_op_ps_madds0,
power_op_ps_madds1,
power_op_ps_merge00,
power_op_ps_merge01,
power_op_ps_merge10,
power_op_ps_merge11,
power_op_ps_mr,
power_op_ps_msub,
power_op_ps_mul,
power_op_ps_muls0,
power_op_ps_muls1,
power_op_ps_nabs,
power_op_ps_neg,
power_op_ps_nmadd,
power_op_ps_nmsub,
power_op_ps_res,
power_op_ps_rsqrte,
power_op_ps_sel,
power_op_ps_sub,
power_op_ps_sum0,
power_op_ps_sum1,
power_op_psq_l,
power_op_psq_lu,
power_op_psq_lux,
power_op_psq_lx,
power_op_psq_st,
power_op_psq_stu,
power_op_psq_stux,
power_op_psq_stx,
power_op_pstb,
power_op_pstd,
power_op_pstfd,
power_op_pstfs,
power_op_psth,
power_op_pstw,
power_op_pstxsd,
power_op_pstxssp,
power_op_pstxv,
power_op_pstxvp,
power_op_qvaligni,
power_op_qvfaligni = power_op_qvaligni,
power_op_qvesplati,
power_op_qvfabs,
power_op_qvfadd,
power_op_qvfadds,
power_op_qvfcfid,
power_op_qvfcfids,
power_op_qvfcfidu,
power_op_qvfcfidus,
power_op_qvfcmpeq,
power_op_qvfcmpgt,
power_op_qvfcmplt,
power_op_qvfcpsgn,
power_op_qvfctid,
power_op_qvfctidu,
power_op_qvfctiduz,
power_op_qvfctidz,
power_op_qvfctiw,
power_op_qvfctiwu,
power_op_qvfctiwuz,
power_op_qvfctiwz,
power_op_qvflogical,
power_op_qvfmadd,
power_op_qvfmadds,
power_op_qvfmr,
power_op_qvfmsub,
power_op_qvfmsubs,
power_op_qvfmul,
power_op_qvfmuls,
power_op_qvfnabs,
power_op_qvfneg,
power_op_qvfnmadd,
power_op_qvfnmadds,
power_op_qvfnmsub,
power_op_qvfnmsubs,
power_op_qvfperm,
power_op_qvfre,
power_op_qvfres,
power_op_qvfrim,
power_op_qvfrin,
power_op_qvfrip,
power_op_qvfriz,
power_op_qvfrsp,
power_op_qvfrsqrte,
power_op_qvfrsqrtes,
power_op_qvfsel,
power_op_qvfsub,
power_op_qvfsubs,
power_op_qvftstnan,
power_op_qvfxmadd,
power_op_qvfxmadds,
power_op_qvfxmul,
power_op_qvfxmuls,
power_op_qvfxxcpnmadd,
power_op_qvfxxcpnmadds,
power_op_qvfxxmadd,
power_op_qvfxxmadds,
power_op_qvfxxnpmadd,
power_op_qvfxxnpmadds,
power_op_qvgpci,
power_op_qvlfcdux,
power_op_qvlfcduxa,
power_op_qvlfcdx,
power_op_qvlfcdxa,
power_op_qvlfcsux,
power_op_qvlfcsuxa,
power_op_qvlfcsx,
power_op_qvlfcsxa,
power_op_qvlfdux,
power_op_qvlfduxa,
power_op_qvlfdx,
power_op_qvlfdxa,
power_op_qvlfiwax,
power_op_qvlfiwaxa,
power_op_qvlfiwzx,
power_op_qvlfiwzxa,
power_op_qvlfsux,
power_op_qvlfsuxa,
power_op_qvlfsx,
power_op_qvlfsxa,
power_op_qvlpcldx,
power_op_qvlpclsx,
power_op_qvlpcrdx,
power_op_qvlpcrsx,
power_op_qvstfcdux,
power_op_qvstfcduxa,
power_op_qvstfcduxi,
power_op_qvstfcduxia,
power_op_qvstfcdx,
power_op_qvstfcdxa,
power_op_qvstfcdxi,
power_op_qvstfcdxia,
power_op_qvstfcsux,
power_op_qvstfcsuxa,
power_op_qvstfcsuxi,
power_op_qvstfcsuxia,
power_op_qvstfcsx,
power_op_qvstfcsxa,
power_op_qvstfcsxi,
power_op_qvstfcsxia,
power_op_qvstfdux,
power_op_qvlstdux = power_op_qvstfdux,
power_op_qvstfduxa,
power_op_qvstfduxi,
power_op_qvlstduxi = power_op_qvstfduxi,
power_op_qvstfduxia,
power_op_qvstfdx,
power_op_qvstfdxa,
power_op_qvstfdxi,
power_op_qvstfdxia,
power_op_qvstfiwx,
power_op_qvstfiwxa,
power_op_qvstfsux,
power_op_qvstfsuxa,
power_op_qvstfsuxi,
power_op_qvstfsuxia,
power_op_qvstfsx,
power_op_qvstfsxa,
power_op_qvstfsxi,
power_op_qvstfsxia,
power_op_rfci,
power_op_rfdi,
power_op_rfebb,
power_op_rfi,
power_op_rfid,
power_op_rfmci,
power_op_rldcl,
power_op_rldcr,
power_op_rldic,
power_op_rldicl,
power_op_rldicr,
power_op_rldimi,
power_op_rlwimi,
power_op_rlwinm,
power_op_rlwnm,
power_op_rotrdi,
power_op_rotrwi,
power_op_sc,
power_op_svcs = power_op_sc,
power_op_setb,
power_op_setbc,
power_op_setbcr,
power_op_setnbc,
power_op_setnbcr,
power_op_slbfee,
power_op_slbia,
power_op_slbie,
power_op_slbieg,
power_op_slbmfee,
power_op_slbmfev,
power_op_slbmte,
power_op_slbsync,
power_op_sld,
power_op_sldi,
power_op_slw,
power_op_slwi,
power_op_srad,
power_op_sradi,
power_op_sraw,
power_op_srawi,
power_op_srd,
power_op_srdi,
power_op_srw,
power_op_srwi,
power_op_stb,
power_op_stbcix,
power_op_stbcx,
power_op_stbepx,
power_op_stbu,
power_op_stbux,
power_op_stbx,
power_op_std,
power_op_stdat,
power_op_stdbrx,
power_op_stdcix,
power_op_stdcx,
power_op_stdcx_rc = power_op_stdcx,
power_op_stdu,
power_op_stdux,
power_op_stdx,
power_op_stfd,
power_op_stfdepx,
power_op_stfdu,
power_op_stfdux,
power_op_stfdx,
power_op_stfiwx,
power_op_stfs,
power_op_stfsu,
power_op_stfsux,
power_op_stfsx,
power_op_sth,
power_op_sthbrx,
power_op_sthcix,
power_op_sthcx,
power_op_sthepx,
power_op_sthu,
power_op_sthux,
power_op_sthx,
power_op_stmw,
power_op_stop,
power_op_stq,
power_op_stqcx,
power_op_stswi,
power_op_stvebx,
power_op_stvehx,
power_op_stvewx,
power_op_stvx,
power_op_stvxl,
power_op_stw,
power_op_stwat,
power_op_stwbrx,
power_op_stwcix,
power_op_stwcx,
power_op_stwcx_rc = power_op_stwcx,
power_op_stwepx,
power_op_stwu,
power_op_stwux,
power_op_stwx,
power_op_stxsd,
power_op_stxsdx,
power_op_stxsibx,
power_op_stxsihx,
power_op_stxsiwx,
power_op_stxssp,
power_op_stxsspx,
power_op_stxv,
power_op_stxvb16x,
power_op_stxvd2x,
power_op_stxvh8x,
power_op_stxvl,
power_op_stxvll,
power_op_stxvp,
power_op_stxvprl,
power_op_stxvprll,
power_op_stxvpx,
power_op_stxvrbx,
power_op_stxvrdx,
power_op_stxvrhx,
power_op_stxvrl,
power_op_stxvrll,
power_op_stxvrwx,
power_op_stxvw4x,
power_op_stxvx,
power_op_subf,
power_op_subfc,
power_op_subfco,
power_op_subfe,
power_op_subfeo,
power_op_subfic,
power_op_subfme,
power_op_subfmeo,
power_op_subfo,
power_op_subfus,
power_op_subfze,
power_op_subfzeo,
power_op_subi,
power_op_subic,
power_op_subis,
power_op_subpcis,
power_op_sync,
power_op_tabort,
power_op_tabortdc,
power_op_tabortdci,
power_op_tabortwc,
power_op_tabortwci,
power_op_tbegin,
power_op_tcheck,
power_op_td,
power_op_tdi,
power_op_tend,
power_op_tlbia,
power_op_tlbie,
power_op_tlbiel,
power_op_tlbivax,
power_op_tlbld,
power_op_tlbli,
power_op_tlbre,
power_op_tlbsx,
power_op_tlbsync,
power_op_tlbwe,
power_op_trap,
power_op_trechkpt,
power_op_treclaim,
power_op_tsr,
power_op_tw,
power_op_twi,
power_op_vabsdub,
power_op_vabsduh,
power_op_vabsduw,
power_op_vaddcuq,
power_op_vaddcuw,
power_op_vaddecuq,
power_op_vaddeuqm,
power_op_vaddfp,
power_op_vaddsbs,
power_op_vaddshs,
power_op_vaddsws,
power_op_vaddubm,
power_op_vaddubs,
power_op_vaddudm,
power_op_vadduhm,
power_op_vadduhs,
power_op_vadduqm,
power_op_vadduwm,
power_op_vadduws,
power_op_vand,
power_op_vandc,
power_op_vavgsb,
power_op_vavgsh,
power_op_vavgsw,
power_op_vavgub,
power_op_vavguh,
power_op_vavguw,
power_op_vbpermd,
power_op_vbpermq,
power_op_vcfsx,
power_op_vcfuged,
power_op_vcfux,
power_op_vcipher,
power_op_vcipherlast,
power_op_vclrlb,
power_op_vclrrb,
power_op_vclzb,
power_op_vclzd,
power_op_vclzdm,
power_op_vclzh,
power_op_vclzlsbb,
power_op_vclzw,
power_op_vcmpbfp,
power_op_vcmpeqfp,
power_op_vcmpequb,
power_op_vcmpequd,
power_op_vcmpequh,
power_op_vcmpequq,
power_op_vcmpequw,
power_op_vcmpgefp,
power_op_vcmpgtfp,
power_op_vcmpgtsb,
power_op_vcmpgtsd,
power_op_vcmpgtsh,
power_op_vcmpgtsq,
power_op_vcmpgtsw,
power_op_vcmpgtub,
power_op_vcmpgtud,
power_op_vcmpgtuh,
power_op_vcmpgtuq,
power_op_vcmpgtuw,
power_op_vcmpneb,
power_op_vcmpneh,
power_op_vcmpnew,
power_op_vcmpnezb,
power_op_vcmpnezh,
power_op_vcmpnezw,
power_op_vcmpsq,
power_op_vcmpuq,
power_op_vcntmbb,
power_op_vcntmbd,
power_op_vcntmbh,
power_op_vcntmbw,
power_op_vctsxs,
power_op_vctuxs,
power_op_vctzb,
power_op_vctzd,
power_op_vctzdm,
power_op_vctzh,
power_op_vctzlsbb,
power_op_vctzw,
power_op_vdivesd,
power_op_vdivesq,
power_op_vdivesw,
power_op_vdiveud,
power_op_vdiveuq,
power_op_vdiveuw,
power_op_vdivsd,
power_op_vdivsq,
power_op_vdivsw,
power_op_vdivud,
power_op_vdivuq,
power_op_vdivuw,
power_op_veqv,
power_op_vexpandbm,
power_op_vexpanddm,
power_op_vexpandhm,
power_op_vexpandqm,
power_op_vexpandwm,
power_op_vexptefp,
power_op_vextddvlx,
power_op_vextddvrx,
power_op_vextdubvlx,
power_op_vextdubvrx,
power_op_vextduhvlx,
power_op_vextduhvrx,
power_op_vextduwvlx,
power_op_vextduwvrx,
power_op_vextractbm,
power_op_vextractd,
power_op_vextractdm,
power_op_vextracthm,
power_op_vextractqm,
power_op_vextractub,
power_op_vextractuh,
power_op_vextractuw,
power_op_vextractwm,
power_op_vextsb2d,
power_op_vextsb2w,
power_op_vextsd2q,
power_op_vextsh2d,
power_op_vextsh2w,
power_op_vextsw2d,
power_op_vextublx,
power_op_vextubrx,
power_op_vextuhlx,
power_op_vextuhrx,
power_op_vextuwlx,
power_op_vextuwrx,
power_op_vgbbd,
power_op_vgnb,
power_op_vinsblx,
power_op_vinsbrx,
power_op_vinsbvlx,
power_op_vinsbvrx,
power_op_vinsd,
power_op_vinsdlx,
power_op_vinsdrx,
power_op_vinsertb,
power_op_vinsertd,
power_op_vinserth,
power_op_vinsertw,
power_op_vinshlx,
power_op_vinshrx,
power_op_vinshvlx,
power_op_vinshvrx,
power_op_vinsw,
power_op_vinswlx,
power_op_vinswrx,
power_op_vinswvlx,
power_op_vinswvrx,
power_op_vlogefp,
power_op_vmaddfp,
power_op_vmaxfp,
power_op_vmaxsb,
power_op_vmaxsd,
power_op_vmaxsh,
power_op_vmaxsw,
power_op_vmaxub,
power_op_vmaxud,
power_op_vmaxuh,
power_op_vmaxuw,
power_op_vmhaddshs,
power_op_vmhraddshs,
power_op_vminfp,
power_op_vminsb,
power_op_vminsd,
power_op_vminsh,
power_op_vminsw,
power_op_vminub,
power_op_vminud,
power_op_vminuh,
power_op_vminuw,
power_op_vmladduhm,
power_op_vmodsd,
power_op_vmodsq,
power_op_vmodsw,
power_op_vmodud,
power_op_vmoduq,
power_op_vmoduw,
power_op_vmrgew,
power_op_vmrghb,
power_op_vmrghh,
power_op_vmrghw,
power_op_vmrglb,
power_op_vmrglh,
power_op_vmrglw,
power_op_vmrgow,
power_op_vmsumcud,
power_op_vmsummbm,
power_op_vmsumshm,
power_op_vmsumshs,
power_op_vmsumubm,
power_op_vmsumudm,
power_op_vmsumuhm,
power_op_vmsumuhs,
power_op_vmul10cuq,
power_op_vmul10ecuq,
power_op_vmul10euq,
power_op_vmul10uq,
power_op_vmulesb,
power_op_vmulesd,
power_op_vmulesh,
power_op_vmulesw,
power_op_vmuleub,
power_op_vmuleud,
power_op_vmuleuh,
power_op_vmuleuw,
power_op_vmulhsd,
power_op_vmulhsw,
power_op_vmulhud,
power_op_vmulhuw,
power_op_vmulld,
power_op_vmulosb,
power_op_vmulosd,
power_op_vmulosh,
power_op_vmulosw,
power_op_vmuloub,
power_op_vmuloud,
power_op_vmulouh,
power_op_vmulouw,
power_op_vmuluwm,
power_op_vnand,
power_op_vncipher,
power_op_vncipherlast,
power_op_vnegd,
power_op_vnegw,
power_op_vnmsubfp,
power_op_vnor,
power_op_vor,
power_op_vorc,
power_op_vpdepd,
power_op_vperm,
power_op_vpermr,
power_op_vpermxor,
power_op_vpextd,
power_op_vpkpx,
power_op_vpksdss,
power_op_vpksdus,
power_op_vpkshss,
power_op_vpkshus,
power_op_vpkswss,
power_op_vpkswus,
power_op_vpkudum,
power_op_vpkudus,
power_op_vpkuhum,
power_op_vpkuhus,
power_op_vpkuwum,
power_op_vpkuwus,
power_op_vpmsumb,
power_op_vpmsumd,
power_op_vpmsumh,
power_op_vpmsumw,
power_op_vpopcntb,
power_op_vpopcntd,
power_op_vpopcnth,
power_op_vpopcntw,
power_op_vprtybd,
power_op_vprtybq,
power_op_vprtybw,
power_op_vrefp,
power_op_vrfim,
power_op_vrfin,
power_op_vrfip,
power_op_vrfiz,
power_op_vrlb,
power_op_vrld,
power_op_vrldmi,
power_op_vrldnm,
power_op_vrlh,
power_op_vrlq,
power_op_vrlqmi,
power_op_vrlqnm,
power_op_vrlw,
power_op_vrlwmi,
power_op_vrlwnm,
power_op_vrsqrtefp,
power_op_vsbox,
power_op_vsel,
power_op_vshasigmad,
power_op_vshasigmaw,
power_op_vsl,
power_op_vslb,
power_op_vsld,
power_op_vsldbi,
power_op_vsldoi,
power_op_vslh,
power_op_vslo,
power_op_vslq,
power_op_vslv,
power_op_vslw,
power_op_vspltb,
power_op_vsplth,
power_op_vspltisb,
power_op_vspltish,
power_op_vspltisw,
power_op_vspltw,
power_op_vsr,
power_op_vsrab,
power_op_vsrad,
power_op_vsrah,
power_op_vsraq,
power_op_vsraw,
power_op_vsrb,
power_op_vsrd,
power_op_vsrdbi,
power_op_vsrh,
power_op_vsro,
power_op_vsrq,
power_op_vsrv,
power_op_vsrw,
power_op_vstribl,
power_op_vstribr,
power_op_vstrihl,
power_op_vstrihr,
power_op_vsubcuq,
power_op_vsubcuw,
power_op_vsubecuq,
power_op_vsubeuqm,
power_op_vsubfp,
power_op_vsubsbs,
power_op_vsubshs,
power_op_vsubsws,
power_op_vsububm,
power_op_vsububs,
power_op_vsubudm,
power_op_vsubuhm,
power_op_vsubuhs,
power_op_vsubuqm,
power_op_vsubuwm,
power_op_vsubuws,
power_op_vsum2sws,
power_op_vsum4sbs,
power_op_vsum4shs,
power_op_vsum4ubs,
power_op_vsumsws,
power_op_vupkhpx,
power_op_vupkhsb,
power_op_vupkhsh,
power_op_vupkhsw,
power_op_vupklpx,
power_op_vupklsb,
power_op_vupklsh,
power_op_vupklsw,
power_op_vxor,
power_op_wait,
power_op_wrtee,
power_op_wrteei,
power_op_xor,
power_op_xori,
power_op_xoris,
power_op_xsabsdp,
power_op_xsabsqp,
power_op_xsadddp,
power_op_xsaddqp,
power_op_xsaddqpo,
power_op_xsaddsp,
power_op_xscmpeqdp,
power_op_xscmpeqqp,
power_op_xscmpexpdp,
power_op_xscmpexpqp,
power_op_xscmpgedp,
power_op_xscmpgeqp,
power_op_xscmpgtdp,
power_op_xscmpgtqp,
power_op_xscmpodp,
power_op_xscmpoqp,
power_op_xscmpudp,
power_op_xscmpuqp,
power_op_xscpsgndp,
power_op_xscpsgnqp,
power_op_xscvdpdp,
power_op_xscvdphp = power_op_xscvdpdp,
power_op_xscvdpqp,
power_op_xscvdpsp,
power_op_xscvdpspn,
power_op_xscvdpsxds,
power_op_xscvdpsxws,
power_op_xscvdpuxds,
power_op_xscvdpuxws,
power_op_xscvhpdp,
power_op_xscvqpdp,
power_op_xscvqpdpo,
power_op_xscvqpsdz,
power_op_xscvqpsqz,
power_op_xscvqpswz,
power_op_xscvqpudz,
power_op_xscvqpuqz,
power_op_xscvqpuwz,
power_op_xscvsdqp,
power_op_xscvspdp,
power_op_xscvspdpn,
power_op_xscvsqqp,
power_op_xscvsxddp,
power_op_xscvsxdsp,
power_op_xscvudqp,
power_op_xscvuqqp,
power_op_xscvuxddp,
power_op_xscvuxdsp,
power_op_xsdivdp,
power_op_xsdivqp,
power_op_xsdivqpo,
power_op_xsdivsp,
power_op_xsiexpdp,
power_op_xsiexpqp,
power_op_xsmaddadp,
power_op_xsmaddasp,
power_op_xsmaddmdp,
power_op_xsmaddmsp,
power_op_xsmaddqp,
power_op_xsmaddqpo,
power_op_xsmaxcdp,
power_op_xsmaxcqp,
power_op_xsmaxdp,
power_op_xsmaxjdp,
power_op_xsmincdp,
power_op_xsmincqp,
power_op_xsmindp,
power_op_xsminjdp,
power_op_xsmsubadp,
power_op_xsmsubasp,
power_op_xsmsubmdp,
power_op_xsmsubmsp,
power_op_xsmsubqp,
power_op_xsmsubqpo,
power_op_xsmuldp,
power_op_xsmulqp,
power_op_xsmulqpo,
power_op_xsmulsp,
power_op_xsnabsdp,
power_op_xsnabsqp,
power_op_xsnegdp,
power_op_xsnegqp,
power_op_xsnmaddadp,
power_op_xsnmaddasp,
power_op_xsnmaddmdp,
power_op_xsnmaddmsp,
power_op_xsnmaddqp,
power_op_xsnmaddqpo,
power_op_xsnmsubadp,
power_op_xsnmsubasp,
power_op_xsnmsubmdp,
power_op_xsnmsubmsp,
power_op_xsnmsubqp,
power_op_xsnmsubqpo,
power_op_xsrdpi,
power_op_xsrdpic,
power_op_xsrdpim,
power_op_xsrdpip,
power_op_xsrdpiz,
power_op_xsredp,
power_op_xsresp,
power_op_xsrqpi,
power_op_xsrqpix,
power_op_xsrqpxp,
power_op_xsrsp,
power_op_xsrsqrtedp,
power_op_xsrsqrtesp,
power_op_xssqrtdp,
power_op_xssqrtqp,
power_op_xssqrtqpo,
power_op_xssqrtsp,
power_op_xssubdp,
power_op_xssubqp,
power_op_xssubqpo,
power_op_xssubsp,
power_op_xstdivdp,
power_op_xstsqrtdp,
power_op_xststdcdp,
power_op_xststdcqp,
power_op_xststdcsp,
power_op_xsxexpdp,
power_op_xsxexpqp,
power_op_xsxsigdp,
power_op_xsxsigqp,
power_op_xvabsdp,
power_op_xvabssp,
power_op_xvadddp,
power_op_xvaddsp,
power_op_xvbf16ger2,
power_op_xvbf16ger2nn,
power_op_xvbf16ger2np,
power_op_xvbf16ger2pn,
power_op_xvbf16ger2pp,
power_op_xvcmpeqdp,
power_op_xvcmpeqsp,
power_op_xvcmpgedp,
power_op_xvcmpgesp,
power_op_xvcmpgtdp,
power_op_xvcmpgtsp,
power_op_xvcpsgndp,
power_op_xvcpsgnsp,
power_op_xvcvbf16spn,
power_op_xvcvdpsp,
power_op_xvcvdpsxds,
power_op_xvcvdpsxws,
power_op_xvcvdpuxds,
power_op_xvcvdpuxws,
power_op_xvcvhpsp,
power_op_xvcvspbf16,
power_op_xvcvspdp,
power_op_xvcvsphp,
power_op_xvcvspsxds,
power_op_xvcvspsxws,
power_op_xvcvspuxds,
power_op_xvcvspuxws,
power_op_xvcvsxddp,
power_op_xvcvsxdsp,
power_op_xvcvsxwdp,
power_op_xvcvsxwsp,
power_op_xvcvuxddp,
power_op_xvcvuxdsp,
power_op_xvcvuxwdp,
power_op_xvcvuxwsp,
power_op_xvdivdp,
power_op_xvdivsp,
power_op_xvf16ger2,
power_op_xvf16ger2nn,
power_op_xvf16ger2np,
power_op_xvf16ger2pn,
power_op_xvf16ger2pp,
power_op_xvf32ger,
power_op_xvf32gernn,
power_op_xvf32gernp,
power_op_xvf32gerpn,
power_op_xvf32gerpp,
power_op_xvf64ger,
power_op_xvf64gernn,
power_op_xvf64gernp,
power_op_xvf64gerpn,
power_op_xvf64gerpp,
power_op_xvi16ger2,
power_op_xvi16ger2pp,
power_op_xvi16ger2s,
power_op_xvi16ger2spp,
power_op_xvi4ger8,
power_op_xvi4ger8pp,
power_op_xvi8ger4,
power_op_xvi8ger4pp,
power_op_xvi8ger4spp,
power_op_xviexpdp,
power_op_xviexpsp,
power_op_xvmaddadp,
power_op_xvmaddasp,
power_op_xvmaddmdp,
power_op_xvmaddmsp,
power_op_xvmaxdp,
power_op_xvmaxsp,
power_op_xvmindp,
power_op_xvminsp,
power_op_xvmsubadp,
power_op_xvmsubasp,
power_op_xvmsubmdp,
power_op_xvmsubmsp,
power_op_xvmuldp,
power_op_xvmulsp,
power_op_xvnabsdp,
power_op_xvnabssp,
power_op_xvnegdp,
power_op_xvnegsp,
power_op_xvnmaddadp,
power_op_xvnmaddasp,
power_op_xvnmaddmdp,
power_op_xvnmaddmsp,
power_op_xvnmsubadp,
power_op_xvnmsubasp,
power_op_xvnmsubmdp,
power_op_xvnmsubmsp,
power_op_xvrdpi,
power_op_xvrdpic,
power_op_xvrdpim,
power_op_xvrdpip,
power_op_xvrdpiz,
power_op_xvredp,
power_op_xvresp,
power_op_xvrspi,
power_op_xvrspic,
power_op_xvrspim,
power_op_xvrspip,
power_op_xvrspiz,
power_op_xvrsqrtedp,
power_op_xvrsqrtesp,
power_op_xvsqrtdp,
power_op_xvsqrtsp,
power_op_xvsubdp,
power_op_xvsubsp,
power_op_xvtdivdp,
power_op_xvtdivsp,
power_op_xvtlsbb,
power_op_xvtsqrtdp,
power_op_xvtsqrtsp,
power_op_xvtstdcdp,
power_op_xvtstdcsp,
power_op_xvxexpdp,
power_op_xvxexpsp,
power_op_xvxsigdp,
power_op_xvxsigsp,
power_op_xxblendvb,
power_op_xxblendvd,
power_op_xxblendvh,
power_op_xxblendvw,
power_op_xxbrd,
power_op_xxbrh,
power_op_xxbrq,
power_op_xxbrw,
power_op_xxeval,
power_op_xxextractuw,
power_op_xxgenpcvbm,
power_op_xxgenpcvdm,
power_op_xxgenpcvhm,
power_op_xxgenpcvwm,
power_op_xxinsertw,
power_op_xxland,
power_op_xxlandc,
power_op_xxleqv,
power_op_xxlnand,
power_op_xxlnor,
power_op_xxlor,
power_op_xxlorc,
power_op_xxlxor,
power_op_xxmfacc,
power_op_xxmrghw,
power_op_xxmrglw,
power_op_xxmtacc,
power_op_xxperm,
power_op_xxpermdi,
power_op_xxpermr,
power_op_xxpermx,
power_op_xxsel,
power_op_xxsetaccz,
power_op_xxsldwi,
power_op_xxsplti32dx,
power_op_xxspltib,
power_op_xxspltidp,
power_op_xxspltiw,
power_op_xxspltw,

