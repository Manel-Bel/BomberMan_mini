#include "../header/debug.h"

//print the formated messages if DEBUG=1
void debug_printf(const char *format, ...){
    // #ifdef DEBUG
        char buf[50];
        va_list args; 
        va_start(args,format);
        vsprintf(buf,format,args);
        va_end(args);
        
        fprintf(stderr,"Debug : %s.\n",buf);
    // #endif
}
