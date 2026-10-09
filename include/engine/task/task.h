#ifndef DTBE_TASK_TASK_H
#define DTBE_TASK_TASK_H

/* The tasks and the scheduler that runs them every frame. */

#include "common.h"
#include "vtable.h"
#include "engine/lib/list.h"
#include "engine/system/handle_table.h"

struct Scheduler;

#ifdef __cplusplus

/*
 * Something run every frame by a Scheduler; the base class of the game's
 * tasks. The C files see it as the struct in the #else part.
 */
class Task {
public:
    /* 0x00 */ ListNode link;
    /* 0x08 */ char *name;
    /* 0x0C */ u32 handle; /* in TASK_HANDLES */
    /* 0x10 */ s32 priority;
    /* 0x14 */ struct Scheduler *scheduler;
    /* 0x18 */ s32 killed;
    /* 0x1C: the virtual table pointer */

    Task(s32 priority, char *name) __asm__("taskInit");
    virtual ~Task();
    virtual void update(s32 arg) __asm__("taskUpdate");
    /* whether killing it leaves it running */
    virtual s32 survives() __asm__("taskSurvives");
};

#else

/* the virtual table of Task, as the C files see it */
typedef struct TaskVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry destroy; /* (Task *, s32 flags) */
    /* 0x10 */ VtableEntry update;  /* (Task *, s32 arg) */
    /* 0x18 */ VtableEntryS32 survives; /* (Task *): whether killing it leaves it running */
} TaskVtable;

/* Something run every frame by a Scheduler; the base class of the game's tasks. */
typedef struct Task {
    /* 0x00 */ ListNode link;
    /* 0x08 */ char *name;
    /* 0x0C */ u32 handle; /* in TASK_HANDLES */
    /* 0x10 */ s32 priority;
    /* 0x14 */ struct Scheduler *scheduler;
    /* 0x18 */ s32 killed;
    /* 0x1C */ TaskVtable *vtable;
} Task;

#endif /* __cplusplus */

/* Runs its tasks every frame and destroys the killed ones the frame after. */
typedef struct Scheduler {
    /* 0x00 */ ListNode tasks;
    /* 0x08 */ ListNode killed;
    /* 0x10 */ s32 count;
    /* 0x14 */ s32 killingAll; /* kill every task, until none is left */
    /* 0x18 */ ListNode *cursor; /* the task to run next */
} Scheduler;

EXTERN_C_BEGIN

#ifndef __cplusplus
/* the virtual table of Task */
extern TaskVtable TASK_VTABLE;

Task *taskInit(Task *task, s32 priority, char *name);
/* Task's destructor, by g++'s name for it */
void taskDestroy(Task *task, s32 flags) __asm__("_._4Task");
s32 taskSurvives(Task *task);
#endif

/* the scheduler that runs the tasks */
extern struct Scheduler SCHEDULER;
/* the handles of the tasks */
extern HandleTable *TASK_HANDLES;

void setTaskHandles(HandleTable *table);
void taskLeaveScheduler(Task *task);
void taskKill(Task *task);
void killTaskByHandle(u32 handle);
Scheduler *schedulerInit(Scheduler *scheduler);
void schedulerDestroy(Scheduler *scheduler, s32 flags);
void schedulerDestroyTasks(Scheduler *scheduler);
void destroyTaskList(ListNode *list);
void schedulerRunTasks(Scheduler *scheduler, s32 arg);
void schedulerRemoveTask(Scheduler *scheduler, Task *task);
Scheduler *schedulerInsertTask(Scheduler *scheduler, Task *task);
void taskUpdate(void);
char *taskGetName(Task *task);
u32 taskGetHandle(Task *task);
s32 taskIsKilled(Task *task);
Task *getTaskByHandle(u32 handle);
Scheduler *taskGetScheduler(Task *task);
void schedulerAddTask(Scheduler *scheduler, Task *task);
void schedulerKillAll(Scheduler *scheduler);
s32 schedulerIsKillingAll(Scheduler *scheduler);
s32 schedulerGetCount(Scheduler *scheduler);

EXTERN_C_END

#endif /* DTBE_TASK_TASK_H */
