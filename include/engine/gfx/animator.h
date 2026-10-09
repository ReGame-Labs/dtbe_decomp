#ifndef DTBE_GFX_ANIMATOR_H
#define DTBE_GFX_ANIMATOR_H

/* Animation files and the animator that plays their clips on nodes, blending poses. */

#include "common.h"
#include "vtable.h"
#include "engine/math/quaternion.h"

EXTERN_C_BEGIN

/* An SVECTOR without its padding. */
typedef struct {
    s16 vx;
    s16 vy;
    s16 vz;
} Vec3s;

/* Where an animation puts a node: a rotation and two vectors. */
typedef struct {
    /* 0x00 */ Quaternion rot;
    /* 0x08 */ Vec3s unk8;
    /* 0x0E */ Vec3s unkE;
} AnimPose; /* size 0x14 */

/* One animation: a key for every node in every frame. */
typedef struct {
    /* 0x0 */ s32 frameCount;
    /* 0x4 */ u32 *keys;
} AnimClip;

/*
 * An animation file. It is loaded with offsets from its start in place of the
 * pointers, which the first Animator to use it turns into pointers. A key packs
 * an index into each of the three tables, the masks and shifts say where.
 */
typedef struct {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s32 nodeCount;
    /* 0x08 */ s16 *parents; /* the parent of each node, < 0 for none */
    /* 0x0C */ s16 *objects; /* the TMD object each node moves */
    /* 0x10 */ s32 clipCount;
    /* 0x14 */ AnimClip *clips;
    /* 0x18 */ Quaternion *rots;
    /* 0x1C */ Vec3s *unk8s;
    /* 0x20 */ Vec3s *unkEs;
    /* 0x24 */ u16 unk24;
    /* 0x26 */ u16 rotMask;
    /* 0x28 */ u16 unk8Shift;
    /* 0x2A */ u16 unk8Mask;
    /* 0x2C */ u16 unkEShift;
    /* 0x2E */ u16 unkEMask;
} AnimData;

/* AnimData.flags: the offsets are pointers already */
#define ANIM_DATA_RELOCATED 0x80000000

typedef struct AnimNodeVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry update; /* (AnimNode *): takes on the new pose */
} AnimNodeVtable;

/* A node an Animator moves: its pose, and the two poses it blends between. */
typedef struct {
    /* 0x00 */ AnimPose pose;
    /* 0x14 */ AnimPose from;
    /* 0x28 */ AnimPose to;
    /* 0x3C */ AnimNodeVtable *vtable;
} AnimNode;

/* Plays the clips of an AnimData on a set of nodes. */
typedef struct {
    /* 0x00 */ AnimData *data;
    /* 0x04 */ AnimNode **nodes;      /* data->nodeCount of them, NULL if unused */
    /* 0x08 */ s32 clip;
    /* 0x0C */ s32 frameCount;        /* of the clip */
    /* 0x10 */ u32 *keys;             /* of the clip */
    /* 0x14 */ s32 frame;
    /* 0x18 */ s32 blendEnd;          /* the frame a blend ends on */
    /* 0x1C */ s32 blendFrames;
    /* 0x20 */ s32 loops;             /* how many more times to play, < 0 forever */
    /* 0x24 */ s32 loopFrame;         /* where a loop starts again */
    /* 0x28 */ u32 flags;
} Animator;

/* Animator.flags */
#define ANIMATOR_STOPPED 1
#define ANIMATOR_ENDED 2 /* the clip came to its end in the last step */

Animator *animatorInit(Animator *animator, AnimData *data);
void animatorDestroy(Animator *animator, s32 flags);
s32 animatorGetClipCount(Animator *animator);
s32 animatorGetNodeCount(Animator *animator);
void animatorSetNode(Animator *animator, s32 i, AnimNode *node);
s16 animatorGetNodeParent(Animator *animator, s32 i);

s16 animatorGetNodeObject(Animator *animator, s32 i);
s32 animatorGetFrameCount(Animator *animator);
s32 animatorGetFrame(Animator *animator);
s32 animatorGetClip(Animator *animator);
s32 animatorIsStopped(Animator *animator);
s32 animatorHasEnded(Animator *animator);
s32 animatorIsClipEmpty(Animator *animator, s32 clip);
void animatorStartClip(Animator *animator, s32 clip, s32 blendFrames, s32 plays, s32 frame);
void animatorJumpToFrame(Animator *animator, s32 frame, s32 blendFrames);
void animatorSetLoopFrame(Animator *animator, s32 frame);
s32 animatorStartBlend(Animator *animator, s32 frame, s32 blendFrames);
void animatorUpdate(Animator *animator);
void animatorUpdateNodes(Animator *animator);
void animatorSetFramePoses(Animator *animator, s32 frame);
void animatorSetBlendPoses(Animator *animator, s32 frame);
void animNodeBlend(AnimNode *node, s32 t);
void blendVec3s(Vec3s *out, Vec3s *from, Vec3s *to, s32 t);
void blendQuaternion(Quaternion *out, Quaternion *from, Quaternion *to, s32 t);

EXTERN_C_END

#endif /* DTBE_GFX_ANIMATOR_H */
