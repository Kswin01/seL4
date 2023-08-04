/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <config.h>
#ifdef CONFIG_ENABLE_BENCHMARKS

#define CCNT "PMCCNTR_EL0"
#define PMCR "PMCR_EL0"
#define PMCNTENSET "PMCNTENSET_EL0"
#define PMINTENSET "PMINTENSET_EL1"
#define PMOVSR "PMOVSCLR_EL0"
#define CCNT_INDEX 31

static inline void armv_enableOverflowIRQ(void)
{
    uint32_t val;
    MRS("PMINTENSET_EL1", val);
    val |= BIT(CCNT_INDEX);
    MSR("PMINTENSET_EL1", val);
    printf("WE ARE ENABLING OVERFLOW IRQ ON ARM8\n");

}

static inline void armv_handleOverflowIRQ(void);
// {
//     printf("This is the thread that caused the fault: %p\n", NODE_STATE(ksCurThread));
//     uint32_t value;

// 	/* Read */
// 	asm volatile("mrs %0, pmovsclr_el0" : "=r" (value));

//     printf("This is the value of the interrupt: %x\n", value);
    
//     uint64_t rr = 0;
//     uint32_t r = 0;

//     asm volatile("isb; mrs %0, pmccntr_el0" : "=r" (rr));
//     printf("This is the current cycle counter: %llu\n", rr);

//     asm volatile("isb; mrs %0, pmevcntr0_el0" : "=r" (r));
//     printf("This is the current event counter 0: %d\n", r);
//     asm volatile("isb; mrs %0, pmevcntr1_el0" : "=r" (r));
//     printf("This is the current event counter 1: %d\n", r);
//     asm volatile("isb; mrs %0, pmevcntr2_el0" : "=r" (r));
//     printf("This is the current event counter 2: %d\n", r);
//     asm volatile("isb; mrs %0, pmevcntr3_el0" : "=r" (r));
//     printf("This is the current event counter 3: %d\n", r);
//     asm volatile("isb; mrs %0, pmevcntr4_el0" : "=r" (r));
//     printf("This is the current event counter 4: %d\n", r);
//     // Halt the PMU
//     asm volatile("msr pmcntenset_el0, %0" :: "r"(0 << 31));

//     uint64_t init_cnt = 0;
//     asm volatile("msr pmccntr_el0, %0" : : "r" (init_cnt));

//     printf("This is the faultip: %lu\n", getRegister(NODE_STATE(ksCurThread), FaultIP));

//     printf("This is the KERNEL PMU IRQ: %d\n", KERNEL_PMU_IRQ);

//     handleFault(ksCurThread);
//     uint32_t val = BIT(CCNT_INDEX);
//     MSR(PMOVSR, val);
// }

#endif /* CONFIG_ENABLE_BENCHMARKS */

