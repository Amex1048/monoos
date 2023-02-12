#include <config.h>
#ifdef DEBUG

#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdint.h>

static inline void dbg_bochs(void)
{
    __asm__ volatile("xchgw %bx, %bx");
}

int32_t init_debug(void);

#endif /* DEBUG_H */

#endif
