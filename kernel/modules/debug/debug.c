#include <config.h>
#ifdef DEBUG

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <hal.h>
#include <string.h>

#include <debug.h>

#include "dbgdef.h"

#ifdef DEBUG_COM
#include <devices/com.h>
#endif

int32_t init_debug(void)
{
#ifdef DEBUG_COM
    return init_serial();
#endif
#ifndef DEBUG_COM
    return -1;
#endif
}

#endif