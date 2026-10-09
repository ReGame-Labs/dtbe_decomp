#ifndef DTBE_SYSTEM_THREAD_H
#define DTBE_SYSTEM_THREAD_H

/* Threads with stacks of their own, switched to and back. */

#include "common.h"

EXTERN_C_BEGIN

/* A thread with its own stack. */
typedef struct {
    /* 0x0 */ s32 id;
    /* 0x4 */ void *stack;
    /* 0x8 */ s32 caller; /* the thread that switched to it */
} Thread;

Thread *threadInit(Thread *thread, void (*func)(), u32 stackSize, void *arg, s32 gp);
void threadDestroy(Thread *thread, s32 flags);
void threadSwitchTo(Thread *thread, s32 value);
void threadSwitchBack(Thread *thread, s32 value);
void switchThread(s32 id, s32 value);
s32 threadGetId(Thread *thread);

EXTERN_C_END

#endif /* DTBE_SYSTEM_THREAD_H */
