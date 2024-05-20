#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include <stdarg.h>
/**
 * @brief Prints a formatted debug message.
 * 
 * This function works similarly to printf, but is intended for debugging purposes.
 * 
 * @param format The format string.
 * @param ... Additional arguments to be formatted.
 */
void debug_printf(const char *format, ...);

#endif
