#include <config.h>
#ifdef DEBUG

#ifndef DBGDEF_H
#define DBGDEF_H

#include <stddef.h>
#include <stdint.h>

// Inits serial port
#ifdef DEBUG_COM
int32_t init_serial(void);
#endif

#endif /* CDBGDEF_H */

#endif