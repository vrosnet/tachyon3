/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"
#include "mboot.h"
#include "spl.h"
#include "rm.h"
#include "extp.h"
#include "mem.h"
#include "spc.h"
#include "process.h"
#include "ksym.h"
#include "intr.h"
#include "kheap.h"

/**
 * the initial state at boot. contains various boot relevant data,
 * saved by the early entry routine
 */
init_state_t const boot_state;

/**
 * The kernel process.
 */
process_t* core;

// -- TESTTEST
void test_thr() {
    while(true) {
        info("hello thread\n");
        for(int i = 0; i <100000; ++i);
    }
}
// -- TESTTEST

void init_subsys(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing %s\n", descr);
    cb();
}

void boot() {
    log_init();

    /* initialize early kernel extensions (before memory is initialized!) */
    extp_iterate(EXTP_EARLY_KINIT, init_subsys);

    pmem_init();

    /* initialize kernel internals registered as extension
     * points in no specific order */
    extp_iterate(EXTP_KINIT, init_subsys);

    /* initialize the BSP. this creates the initial cpu context and state */
    extp_iterate(EXTP_CPUINIT, init_subsys);

    /* initialize the core process with the current address
     * space, and other relevant data. */
    core = prc_new(spc_current());

    if(!core)
        fatal("failed to create core process\n");

    thread_t* hi_thread = thr_create(core, test_thr);
    thr_switch(hi_thread);

    /*
    rm_state_t state;
    memset(&state, 0, sizeof(state));
    state.ax.word = 0x4F00;

    if(!rm_int(0x10, &state))
        warn("failed calling int 0x15\n");
    */

    info("kheap: used bytes: %d, allocated blocks: %d\n", kheap.state.used_bytes, kheap.state.block_count);

    asm volatile("int $0x20");

    fatal("kernel ended unexpectedly.\n");
}
