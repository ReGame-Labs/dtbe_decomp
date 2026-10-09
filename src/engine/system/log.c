#include "common.h"
#include "engine/system/log.h"
#include "stdio.h"

LogFunc LOG_PRINT = (LogFunc)printf;

/* Turns the log on or off. */
void setLogEnabled(s32 on) {
    if (!on) {
        /* the no-op takes no arguments, which a caller may pass anyway */
        LOG_PRINT = (LogFunc)printNothing;
    } else {
        LOG_PRINT = (LogFunc)printf;
    }
}

/* The log function when the log is off: does nothing. */
void printNothing(void) {
}
