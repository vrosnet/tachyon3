/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "uapi.h"
#include "vmem_mgmt.h"
#include "ldsym.h"
#include "paging.h"

static uapi_desc_t const SECTION(SECT_USER_DATA) uapi_desc = {
    .syscall=uapi_sysc_call
};

uintptr_t SECTION(SECT_USER_CODE) uapi_sysc_call(syscall_t call, uintptr_t p0, uintptr_t p1) {
    uintptr_t res;

    // TODO: sysenter, etc.?
    
    asm volatile(
        "\tint %1\n"
        "\tmov %%rax, %0\n"
        : "=a"(res) 
        : "i"(SYSC_INTERRUPT), "D"(call), "S"(p0), "d"(p1));

    return res;
}

void SECTION(SECT_USER_CODE) uapi_thr_trampoline(thread_t* thread, thread_start_t entry) {
    entry(&uapi_desc);

    thread->state = Exited;
    sysc_call(SysSchedule, 0, 0);

    /* never reached - as the thread is aborting, it will never be re-scheduled */
}

static bool _uapi_map_from_to(uintptr_t* sv, uintptr_t* sp, uintptr_t* ev, uintptr_t* ep) {
    // attention: keep sv and sp synchronous!
    while(sv < ev && sp < ep) {
        vmem_mgmt_add_global_mapping((uintptr_t)sp, sv, PG_USER | PG_GLOBAL);
        
        sv += PAGE_SIZE_4K;
        sp += PAGE_SIZE_4K;
    }

    return true;
}

void uapi_init() {
    _uapi_map_from_to(&_core_vma_user_code, &_core_lma_user_code, &_core_vma_user_ecode, &_core_lma_user_ecode);
    _uapi_map_from_to(&_core_vma_user_data, &_core_lma_user_data, &_core_vma_user_edata, &_core_lma_user_edata);
}

