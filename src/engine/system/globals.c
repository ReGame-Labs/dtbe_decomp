#include "common.h"
#include "engine/system/globals.h"
#include "engine/game/game_state.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/lib/list.h"
#include "engine/math/random.h"
#include "engine/sound/sound_control.h"
#include "engine/task/task.h"
#include "vtable.h"

/*
 * Builds (initialize) or destroys the global objects: the scheduler, the
 * random numbers, the ordering table, the sound task, the fade task and the
 * game state. It is g++'s __static_initialization_and_destruction_0 of this
 * file, written in C: built by cc1plus from the objects themselves, the
 * inlined destructor of the sound task reads seps.prev through the list's
 * address and not the task's, a load the game's does not have.
 */
void initOrDestroyGlobals(s32 initialize, s32 priority) {
    SoundControl *sound;
    ListNode *seps;
    ListNode *vabs;

    if (priority == INIT_PRIORITY_DEFAULT) {
        if (initialize) {
            schedulerInit(&SCHEDULER);
            mersenneTwisterInit(&RANDOM);
            orderingTableInit(&FRAME_OT, 8);
            soundControlInit(&SOUND_CONTROL);
            fadeControlInit(&FADE_CONTROL);
            gameStateInit(&GAME_STATE);
            return;
        }
        taskDestroy(&FADE_CONTROL.task, DESTROY_BASES);
        /* the sound task's destructor, inlined: listRemove of both lists, in
         * the access order the game's compiler used */
        sound = &SOUND_CONTROL;
        seps = &sound->seps;
        sound->seps.next->prev = sound->seps.prev;
        seps->prev->next = sound->seps.next;
        sound->seps.next = seps;
        sound->seps.prev = seps;
        vabs = &sound->vabs;
        sound->vabs.next->prev = sound->vabs.prev;
        vabs->prev->next = sound->vabs.next;
        sound->vabs.next = vabs;
        sound->vabs.prev = vabs;
        /* SoundControl starts with a Task (see engine/sound/sound_control.h) */
        taskDestroy((Task *)sound, DESTROY_BASES);
        schedulerDestroy(&SCHEDULER, DESTROY_BASES);
    }
}

/* Builds the global objects: g++'s _GLOBAL_.I of this file, the first of
 * STATIC_CONSTRUCTORS. */
void initGlobals(void) {
    initOrDestroyGlobals(1, INIT_PRIORITY_DEFAULT);
}

/* Destroys the global objects: g++'s _GLOBAL_.D of this file, the first of
 * STATIC_DESTRUCTORS. */
void destroyGlobals(void) {
    initOrDestroyGlobals(0, INIT_PRIORITY_DEFAULT);
}
