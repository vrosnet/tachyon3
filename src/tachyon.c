/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"
#include "ldsym.h"
#include "mboot.h"
#include "spl.h"
#include "rm.h"
#include "extp.h"
#include "mem.h"

init_state_t const boot_state;

void init_subsys(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing %s\n", descr);
    cb();
}

void boot() {
    log_init();
    pmem_init();

    /* reserve the kernel's physical memory, so nobody
     * else tries to use it */
    if(!pmem_reserve(0xA0000, (((size_t)&_core_lma_end) - 0xA0000))) {
        error("failed to protect physical lower and kernel memory\n");
    }

    /* initialize kernel internals registered as extension
     * points in no specific order */
    extp_iterate(EXTP_KINIT, init_subsys);

    rm_state_t state;
    memset(&state, 0, sizeof(state));
    state.ax.word = 0x4F00;

    if(!rm_int(0x10, &state))
        warn("failed calling int 0x15\n");

    fatal("kernel ended unexpectedly.\n");
}
