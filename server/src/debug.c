#include "../header/debug.h"
#define DEBUG 0
// print the formated messages if DEBUG=1
void debug_printf(const char *format, ...)
{
    // #ifdef DEBUG
    if (DEBUG)
    {

        char buf[50];
        va_list args;
        va_start(args, format);
        vsprintf(buf, format, args);
        va_end(args);

        printf("Debug : %s.\n", buf);
    }
    // #endif
}
