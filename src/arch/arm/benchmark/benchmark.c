/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <benchmark/benchmark.h>
#include <arch/benchmark.h>
#include <armv/benchmark.h>
#include <api/faults.h>
#include <arch/arm/arch/64/mode/kernel/vspace.h>

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

    uint32_t mask = 0;

    /* Disable Performance Counter */
    // MRS("PMCR_EL0", value);
    mask = 0;
    mask |= (1 << 0); /* Enable */
    mask |= (1 << 1); /* Cycle counter reset */
    mask |= (1 << 2); /* Reset all counters */
    MSR("PMCR_EL0", (~mask));

    /* Disable cycle counter register */
    // MRS("PMCNTENSET_EL0", value);
    mask = 0;
    mask |= (1 << 31);
    MSR("PMCNTENSET_EL0", (~mask));

    // Get the PC 
    uint64_t pc = getRegister(NODE_STATE(ksCurThread), FaultIP);
    // Save the interrupt flags
    uint32_t irq_f = 0;
    MRS(PMOVSR, irq_f);
    uint32_t val = BIT(CCNT_INDEX);
    MSR(PMOVSR, val);

    // Unwinding the call stack, currently only supporting 4 prev calls (arbitrary size)
    /* NOTES        
        
        The frame pointer is saved in register x29.
        
        Will need a way to detect if frame pointers have been saved??
        */

    // Arch_userStackTrace(ksCurThread);

    // word_t fp = getRegister(NODE_STATE(ksCurThread), X29);
    // printf("This is the frame pointer: %lx\n", fp);

    // CALL STACK UNWINDING

    // First, get the threadRoot capability based on the current tcb
    cap_t threadRoot = TCB_PTR_CTE_PTR(NODE_STATE(ksCurThread), tcbVTable)->cap;

    /* lookup the vspace root */
    if (cap_get_capType(threadRoot) != cap_vtable_root_cap) {
        printf("Invalid vspace\n");
        return;
    }

    vspace_root_t *vspaceRoot = cap_vtable_root_get_basePtr(threadRoot);

    // Read the x29 register for the address of the current frame pointer
    word_t fp = getRegister(NODE_STATE(ksCurThread), X29);

    // Loop and read the start of the frame pointer, save the lr value and load the next fp
    for (int i = 0; i < 4; i++) {
        // The LR should be above the FP
        word_t lr_addr = fp + sizeof(word_t);

        // We need to traverse the list. We want to save the value of the LR in the frame
        // entry, and then look at the next frame record. 
        readWordFromVSpace_ret_t read_lr = readWordFromVSpace(vspaceRoot, lr_addr);
        readWordFromVSpace_ret_t read_fp = readWordFromVSpace(vspaceRoot, fp);
        if (read_fp.status == EXCEPTION_NONE && read_lr.status == EXCEPTION_NONE) {
            // Set the fp value to the next frame entry
            fp = read_fp.value;

            // If the fp is 0, then we have reached the end of the frame stack chain
            if (fp == 0) {
                break;
            }


        } else {
            // If we are unable to read, then we have reached the end of our stack unwinding
            printf("0x%"SEL4_PRIx_word": INVALID\n",
                   lr_addr);
            break;
        }
        
    } 

    current_fault = seL4_Fault_PMUEvent_new(pc, irq_f);
    
    if (isRunnable(NODE_STATE(ksCurThread))) {
        handleFault(NODE_STATE(ksCurThread));
        schedule();
        activateThread();
    }

}

#endif
