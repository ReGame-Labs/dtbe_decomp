#include "common.h"
#include "engine/task/task.h"
#include "engine/lib/list.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* Sets the table of the tasks' handles. */
void setTaskHandles(HandleTable *table) {
    TASK_HANDLES = table;
}

/* Constructs a task, alone in its list, with a handle; makes the handle table if there is none. */
Task *taskInit(Task *this, s32 priority, char *name) {
    this->vtable = &TASK_VTABLE;
    this->link.next = &this->link;
    this->link.prev = &this->link;
    if (TASK_HANDLES == NULL) {
        setTaskHandles(handleTableInit(operatorNew(sizeof(HandleTable)), 8));
    }
    this->name = name;
    this->handle = handleTableAdd(TASK_HANDLES, this);
    this->priority = priority;
    this->killed = 0;
    return this;
}

/* Destroys a task: frees its handle and takes it out of its scheduler and list. */
void taskDestroy(Task *this, s32 flags) {
    this->vtable = &TASK_VTABLE;
    handleTableRemove(TASK_HANDLES, this->handle);
    taskLeaveScheduler(this);
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Takes a task out of its scheduler's count. */
void taskLeaveScheduler(Task *this) {
    schedulerRemoveTask(this->scheduler, this);
    this->scheduler = NULL;
}

/* Whether killing the task leaves it running: no (virtual). */
s32 taskSurvives(Task *this) {
    return 0;
}

/* Kills a task: its scheduler destroys it. */
void taskKill(Task *this) {
    this->killed = 1;
}

/* Kills the task of a handle, if it is still there. */
void killTaskByHandle(u32 handle) {
    Task *task = handleTableGet(TASK_HANDLES, handle);

    if (task != NULL) {
        taskKill(task);
    }
}

/* Constructs a scheduler with no tasks. */
Scheduler *schedulerInit(Scheduler *this) {
    this->tasks.next = &this->tasks;
    this->tasks.prev = &this->tasks;
    this->killed.next = &this->killed;
    this->killed.prev = &this->killed;
    this->count = 0;
    this->killingAll = 0;
    this->cursor = &this->tasks;
    return this;
}

/* Destroys a scheduler and its tasks. */
void schedulerDestroy(Scheduler *this, s32 flags) {
    ListNode *killed;

    schedulerDestroyTasks(this);
    killed = &this->killed;
    this->killed.next->prev = this->killed.prev;
    killed->prev->next = this->killed.next;
    this->killed.next = killed;
    this->killed.prev = killed;
    this->tasks.next->prev = this->tasks.prev;
    this->tasks.prev->next = this->tasks.next;
    this->tasks.next = &this->tasks;
    this->tasks.prev = &this->tasks;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Destroys every task of a scheduler, running or killed. */
void schedulerDestroyTasks(Scheduler *this) {
    destroyTaskList(&this->tasks);
    destroyTaskList(&this->killed);
}

/* Destroys every task of a list (a task starts with its link). */
void destroyTaskList(ListNode *list) {
    ListNode *node;
    ListNode *next;
    Task *task;

    for (node = list->next; node != list; node = next) {
        next = node->next;
        task = (Task *)node;
        if (task != NULL) {
            task->vtable->destroy.func((u8 *)task + task->vtable->destroy.delta, DESTROY_DELETE);
        }
    }
}

/*
 * Destroys the tasks killed last frame, then runs every task once with arg,
 * moving the killed ones to the killed list.
 */
void schedulerRunTasks(Scheduler *this, s32 arg) {
    ListNode *killed = &this->killed;
    Task *task;

    destroyTaskList(killed);
    for (task = (Task *)this->tasks.next; &task->link != &this->tasks; task = (Task *)this->cursor) {
        this->cursor = task->link.next;
        task->vtable->update.func((u8 *)task + task->vtable->update.delta, arg);
        if ((this->killingAll || task->killed) &&
            !task->vtable->survives.func((u8 *)task + task->vtable->survives.delta)) {
            listRemove(&task->link);
            task->link.next = killed;
            task->link.prev = this->killed.prev;
            killed->prev->next = &task->link;
            this->killed.prev = &task->link;
        }
    }
    if (this->killingAll && this->count == 0) {
        this->killingAll = 0;
    }
}

/* Takes a task out of a scheduler's count, moving its cursor past the task. */
void schedulerRemoveTask(Scheduler *this, Task *task) {
    if (&task->link == this->cursor) {
        this->cursor = task->link.next;
    }
    this->count--;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/task/task", TASK_VTABLE);

/*
 * Adds a task to a scheduler. It goes after the last task when its priority is
 * not below that one's, else first: the search walks on from the last task.
 */
Scheduler *schedulerInsertTask(Scheduler *this, Task *task) {
    ListNode *node;

    for (node = this->tasks.prev; node != &this->tasks; node = node->next) {
        /* link is the first member of a Task */
        if (((Task *)node)->priority <= task->priority) {
            break;
        }
    }
    listInsertAfter(node, &task->link);
    task->scheduler = this;
    this->count++;
    return this;
}

/* Does nothing. */
void taskUpdate(void) {
}

/* Returns the name of a task. */
char *taskGetName(Task *this) {
    return this->name;
}

/* Returns the handle of a task. */
u32 taskGetHandle(Task *this) {
    return this->handle;
}

/* Returns whether a task was killed. */
s32 taskIsKilled(Task *this) {
    return this->killed;
}

/* Returns the task of a handle, or NULL if it is gone. */
Task *getTaskByHandle(u32 handle) {
    return handleTableGet(TASK_HANDLES, handle);
}

/* Returns the scheduler of a task. */
Scheduler *taskGetScheduler(Task *this) {
    return this->scheduler;
}

/* Adds a task to a scheduler. */
void schedulerAddTask(Scheduler *this, Task *task) {
    schedulerInsertTask(this, task);
}

/* Kills every task of a scheduler. */
void schedulerKillAll(Scheduler *this) {
    this->killingAll = 1;
}

/* Returns whether a scheduler is killing all its tasks. */
s32 schedulerIsKillingAll(Scheduler *this) {
    return this->killingAll;
}

/* Returns the number of tasks of a scheduler. */
s32 schedulerGetCount(Scheduler *this) {
    return this->count;
}
