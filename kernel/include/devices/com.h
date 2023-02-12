#include <config.h>
#ifdef DEBUG_COM

#ifndef COM_H
#define COM_H

#include <stddef.h>
#include <stdint.h>

// Sends char via serial port
int32_t com_send_char(char c);

#endif /* COM_H */

#endif