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
    printf("This is the thread that caused the fault: %p\n", NODE_STATE(ksCurThread));
    uint32_t value;

	/* Read */
	asm volatile("mrs %0, pmovsclr_el0" : "=r" (value));

    printf("This is the value of the interrupt: %x\n", value);
    
    uint64_t rr = 0;
    uint32_t r = 0;

    asm volatile("isb; mrs %0, pmccntr_el0" : "=r" (rr));
    printf("This is the current cycle counter: %llu\n", rr);

    asm volatile("isb; mrs %0, pmevcntr0_el0" : "=r" (r));
    printf("This is the current event counter 0: %d\n", r);
    asm volatile("isb; mrs %0, pmevcntr1_el0" : "=r" (r));
    printf("This is the current event counter 1: %d\n", r);
    asm volatile("isb; mrs %0, pmevcntr2_el0" : "=r" (r));
    printf("This is the current event counter 2: %d\n", r);
    asm volatile("isb; mrs %0, pmevcntr3_el0" : "=r" (r));
    printf("This is the current event counter 3: %d\n", r);
    asm volatile("isb; mrs %0, pmevcntr4_el0" : "=r" (r));
    printf("This is the current event counter 4: %d\n", r);
    // Halt the PMU
    asm volatile("msr pmcntenset_el0, %0" :: "r"(0 << 31));

    uint64_t init_cnt = 0;
    asm volatile("msr pmccntr_el0, %0" : : "r" (init_cnt));
    uint64_t ip = getRegister(NODE_STATE(ksCurThread), FaultIP);
    printf("This is the faultip: %llu\n", ip);
    printf("This is the KERNEL PMU IRQ: %d\n", KERNEL_PMU_IRQ);
    uint32_t val = BIT(CCNT_INDEX);
    MSR(PMOVSR, val);

    current_fault = seL4_Fault_PMUEvent_new(ip);
    
    assert(isRunnable(NODE_STATE(ksCurThread)));
        if (isRunnable(NODE_STATE(ksCurThread))) {
            handleFault(NODE_STATE(ksCurThread));
        }

}

#endif
