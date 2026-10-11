#include "common.h"
#include "engine/system/thread.h"
#include "engine/system/memory.h"
#include "kernel.h"
#include "libapi.h"
#include "vtable.h"

/* the kernel's table of the thread control blocks (SysToT[2].head), read from
 * where the kernel keeps it */
#define TCB_TABLE (*(struct TCB **)0x110)
/* the part of a thread id that indexes the table */
#define THREAD_INDEX(id) ((id) & 0xFFFF)

/* Builds the thread. It stays in asm: when given no gp, it gives the thread
 * its own $gp (or $s1, $zero, $gp), which C can't read without inline asm. */
INCLUDE_ASM("main/nonmatchings/system/thread", threadInit);

/* Destroys the thread and frees its stack. */
void threadDestroy(Thread *this, s32 flags) {
    EnterCriticalSection();
    CloseTh(this->id);
    ExitCriticalSection();
    if (this->stack != NULL) {
        operatorVecDelete(this->stack);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* the thread running: at first the main thread */
static s32 CURRENT_THREAD = DescTH;

/* Switches to the thread, handing it value; it switches back to the thread
 * running now. Returns the value it switches back with. */
s32 threadSwitchTo(Thread *this, s32 value) {
    this->caller = CURRENT_THREAD;
    return switchThread(this->id, value);
}

/* Switches back to the thread that switched to this one, handing it value.
 * Returns the value of the next switch to this thread. */
s32 threadSwitchBack(Thread *this, s32 value) {
    return switchThread(this->caller, value);
}

/* Switches to thread id, which gets value as what its switch returned. Returns
 * what the switch back to this thread hands it (ChangeTh returns the v0 that
 * switch stored in this thread's TCB). */
s32 switchThread(s32 id, s32 value) {
    struct TCB *tcb = TCB_TABLE;

    tcb += THREAD_INDEX(id);
    tcb->reg[R_V0] = value;
    CURRENT_THREAD = id;
    return ChangeTh(id);
}

/* The id of the thread. */
s32 threadGetId(Thread *this) {
    return this->id;
}
