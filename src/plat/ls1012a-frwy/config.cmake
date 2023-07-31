#
# Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
# Copyright 2022, Capgemini Engineering
#
# SPDX-License-Identifier: GPL-2.0-only
#

cmake_minimum_required(VERSION 3.7.2)

declare_platform(ls1012a-frwy KernelPlatformLs1012a-frwy PLAT_LS1012A_FRWY KernelSel4ArchAarch64)

if(KernelPlatformLs1012a-frwy)
    declare_seL4_arch(aarch64)
    set(KernelArmCortexA53 ON)
    set(KernelArchArmV8a ON)
    config_set(KernelARMPlatform ARM_PLAT ls1012a-frwy)
    list(APPEND KernelDTSList "tools/dts/ls1012a-frwy.dts")
    list(APPEND KernelDTSList "src/plat/ls1012a-frwy/overlay-ls1012a-frwy.dts")
    declare_default_headers(
        TIMER_FREQUENCY 125000000
        MAX_IRQ 288
        NUM_PPI 32
        TIMER drivers/timer/arm_generic.h
        INTERRUPT_CONTROLLER arch/machine/gic_v2.h
        CLK_MAGIC 1llu
        CLK_SHIFT 8u
        KERNEL_WCET 10u
    )
endif()

add_sources(
DEP "KernelPlatformLs1012a-frwy"
CFILES src/arch/arm/machine/gic_v2.c src/arch/arm/machine/l2c_nop.c
)