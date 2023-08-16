/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <benchmark/benchmark.h>
#include <arch/benchmark.h>
#include <armv/benchmark.h>
#include <api/faults.h>
#if CONFIG_MAX_NUM_TRACE_POINTS > 0
timestamp_t ksEntries[CONFIG_MAX_NUM_TRACE_POINTS];
bool_t ksStarted[CONFIG_MAX_NUM_TRACE_POINTS];
timestamp_t ksExit;
seL4_Word ksLogIndex = 0;
seL4_Word ksLogIndexFinalized = 0;
#endif /* CONFIG_MAX_NUM_TRACE_POINTS > 0 */

#ifdef CONFIG_ARM_ENABLE_PMU_OVERFLOW_INTERRUPT
UP_STATE_DEFINE(uint64_t, ccnt_num_overflows);
#endif /* CONFIG_ARM_ENABLE_PMU_OVERFLOW_INTERRUPT */

#ifdef CONFIG_ENABLE_BENCHMARKS
void arm_init_ccnt(void)
{

    uint32_t val = (BIT(PMCR_ENABLE) | BIT(PMCR_CCNT_RESET) | BIT(PMCR_ECNT_RESET));
    SYSTEM_WRITE_WORD(PMCR, val);

#ifdef PMCNTENSET
    /* turn on the cycle counter */
    SYSTEM_WRITE_WORD(PMCNTENSET, BIT(CCNT_INDEX));
#endif

#ifdef KERNEL_PMU_IRQ
    printf("KERNEL PMU IRQ is defined\n");
    printf("This is the value of KERNEL PMU IRQ: %d\n", KERNEL_PMU_IRQ);
#endif
#ifdef CONFIG_BENCHMARK_TRACK_UTILISATION
    printf("BENCHMARK TRACK UTIL is defined\n");
#endif

    printf("Attempting to enable arm pmu overflow\n");
#ifdef CONFIG_ARM_ENABLE_PMU_OVERFLOW_INTERRUPT
    printf("CONFIG ARM ENABLE PMU OVERFLOW INTERRUPT HAS BEEN ENABLED\n");
    armv_enableOverflowIRQ();
#endif /* CONFIG_ARM_ENABLE_PMU_OVERFLOW_INTERRUPT */
}
#endif

#ifdef CONFIG_ENABLE_BENCHMARKS
void armv_handleOverflowIRQ(void) {
    // Halt the PMU
    // SYSTEM_WRITE_WORD(PMCNTENSET, (0 << 31));
    // uint64_t val2 = 0;
    // asm volatile("MRS %0, pmcr_el0" : "=r" (val2));
    // val2 &= 0x3f;
    // asm volatile("isb" : : : "memory");
    // asm volatile("MSR pmcr_el0, %0" : : "r" (val2));
    uint32_t value = 0;
    uint32_t mask = 0;

    /* Disable Performance Counter */
    asm volatile("MRS %0, PMCR_EL0" : "=r" (value));
    mask = 0;
    mask |= (1 << 0); /* Enable */
    mask |= (1 << 1); /* Cycle counter reset */
    mask |= (1 << 2); /* Reset all counters */
    asm volatile("MSR PMCR_EL0, %0" : : "r" (value & ~mask));

    /* Disable cycle counter register */
    asm volatile("MRS %0, PMCNTENSET_EL0" : "=r" (value));
    mask = 0;
    mask |= (1 << 31);
    asm volatile("MSR PMCNTENSET_EL0, %0" : : "r" (value & ~mask));

    // Get the PC 
    uint64_t pc = getRegister(NODE_STATE(ksCurThread), FaultIP);
    // Save the interrupt flags
    uint32_t irq_f = 0;
    MRS(PMOVSR, irq_f);
    uint32_t val = BIT(CCNT_INDEX);
    MSR(PMOVSR, val);

    // Unwinding the call stack, currently only supporting 4 prev calls (arbitrary size)
    /* NOTES
        The target programs will require compiling with the arm "-fno-omit-frame-pointer" option.
        
        This forces the compiler to add in frame pointers on every function call, and is required
        for stack unwinding. 
        
        The frame pointer is saved in register x29.
        
        Will need a way to detect if frame pointers have been saved??
        */

    // Arch_userStackTrace(ksCurThread);

    // word_t fp = getRegister(NODE_STATE(ksCurThread), X29);
    // printf("This is the frame pointer: %lx\n", fp);

    current_fault = seL4_Fault_PMUEvent_new(pc, irq_f);
    
    assert(isRunnable(NODE_STATE(ksCurThread)));

    if (isRunnable(NODE_STATE(ksCurThread))) {
    handleFault(NODE_STATE(ksCurThread));
    }

}

#endif
