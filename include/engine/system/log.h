#ifndef DTBE_SYSTEM_LOG_H
#define DTBE_SYSTEM_LOG_H

/* The log of the CD code: printf, or a function that prints nothing when it is off. */

#include "common.h"

EXTERN_C_BEGIN

/* a printf-like log function */
typedef void (*LogFunc)(const char *format, ...);

/* the log function of the CD code: printf, or printNothing when the log is
 * off (setLogEnabled) */
extern LogFunc LOG_PRINT;

void setLogEnabled(s32 on);
void printNothing(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_LOG_H */
