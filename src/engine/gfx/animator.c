#include "common.h"
#include "engine/gfx/animator.h"
#include "engine/math/quaternion.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* The address of what offset, counted from the start of data, points at. */
#define RELOCATE(data, offset) ((s32)(offset) + (s32)(data))

/* Builds an animator for data, with no nodes and stopped. */
Animator *animatorInit(Animator *this, AnimData *data) {
    AnimClip *clip;
    AnimClip *end;
    AnimNode **node;
    AnimNode **nodesEnd;

    this->data = data;
    if (!(data->flags & ANIM_DATA_RELOCATED)) {
        data->flags |= ANIM_DATA_RELOCATED;
        data->parents = (s16 *)RELOCATE(data, data->parents);
        data->objects = (s16 *)RELOCATE(data, data->objects);
        data->clips = (AnimClip *)RELOCATE(data, data->clips);
        data->rots = (Quaternion *)RELOCATE(data, data->rots);
        data->unk8s = (Vec3s *)RELOCATE(data, data->unk8s);
        data->unkEs = (Vec3s *)RELOCATE(data, data->unkEs);
        for (clip = data->clips, end = clip + data->clipCount; clip != end; clip++) {
            clip->keys = (u32 *)RELOCATE(data, clip->keys);
        }
    }
    this->nodes = (AnimNode **)operatorVecNew(data->nodeCount * sizeof(AnimNode *));
    for (node = this->nodes, nodesEnd = node + data->nodeCount; node != nodesEnd; node++) {
        *node = NULL;
    }
    this->frame = 0;
    this->blendFrames = 0;
    this->loops = 0;
    this->flags = ANIMATOR_STOPPED;
    return this;
}

/* Destroys the animator, and frees it if flags has DESTROY_FREE. */
void animatorDestroy(Animator *this, s32 flags) {
    if (this->nodes != NULL) {
        operatorVecDelete(this->nodes);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* How many clips the data has. */
s32 animatorGetClipCount(Animator *this) {
    return this->data->clipCount;
}

/* How many nodes the data animates. */
s32 animatorGetNodeCount(Animator *this) {
    return this->data->nodeCount;
}

/* Gives the animator node i. */
void animatorSetNode(Animator *this, s32 i, AnimNode *node) {
    this->nodes[i] = node;
}

/* The parent of node i, < 0 for none. */
s16 animatorGetNodeParent(Animator *this, s32 i) {
    return this->data->parents[i];
}

/* The TMD object node i moves. */
s16 animatorGetNodeObject(Animator *this, s32 i) {
    return this->data->objects[i];
}

/* How many frames the clip has. */
s32 animatorGetFrameCount(Animator *this) {
    return this->frameCount;
}

/* The frame the animator is on. */
s32 animatorGetFrame(Animator *this) {
    return this->frame;
}

/* The clip the animator plays. */
s32 animatorGetClip(Animator *this) {
    return this->clip;
}

/* Whether the animator has stopped. */
s32 animatorIsStopped(Animator *this) {
    return this->flags & ANIMATOR_STOPPED;
}

/* Whether the clip came to its end in the last step. */
s32 animatorHasEnded(Animator *this) {
    return (this->flags >> 1) & 1;
}

/* Whether clip has no frames. */
s32 animatorIsClipEmpty(Animator *this, s32 clip) {
    AnimClip *entry = &this->data->clips[clip];

    return entry->frameCount == 0;
}

/*
 * Starts clip at frame, blending into it over blendFrames, to play plays times
 * (0 or less for ever). A negative clip stops the animator.
 */
void animatorStartClip(Animator *this, s32 clip, s32 blendFrames, s32 plays, s32 frame) {
    AnimData *data = this->data;
    AnimClip *entry;

    this->clip = clip;
    if (clip < 0) {
        this->flags |= ANIMATOR_STOPPED;
        return;
    }
    entry = &data->clips[clip];
    this->frameCount = entry->frameCount;
    this->keys = entry->keys;
    this->flags = 0;
    this->loopFrame = 0;
    this->loops = plays - 1;
    this->frame = animatorStartBlend(this, frame, blendFrames);
}

/* Jumps to frame, blending into it over blendFrames, unless stopped. */
void animatorJumpToFrame(Animator *this, s32 frame, s32 blendFrames) {
    if (!(this->flags & ANIMATOR_STOPPED)) {
        this->frame = animatorStartBlend(this, frame, blendFrames);
    }
}

/* Makes loops start again at frame. */
void animatorSetLoopFrame(Animator *this, s32 frame) {
    this->loopFrame = frame;
}

/*
 * Sets the nodes up to blend from their pose to the one of frame over
 * blendFrames. A negative frame counts from the end. Returns the frame the
 * blend starts on.
 */
s32 animatorStartBlend(Animator *this, s32 frame, s32 blendFrames) {
    AnimData *data;
    u32 *keys;
    Quaternion *rots;
    Vec3s *unk8s;
    Vec3s *unkEs;
    AnimNode *node;
    s32 nodeCount;
    s32 i;
    u32 key;
    u32 rot;
    u32 unk8;
    u32 unkE;

    if (frame > this->frameCount - 1) {
        frame = this->frameCount - 1;
    } else if (frame < 0) {
        frame += this->frameCount;
        if (frame < 0) {
            frame = 0;
        }
    }
    this->blendEnd = frame;
    this->blendFrames = blendFrames;
    if (blendFrames <= 0) {
        return frame;
    }
    data = this->data;
    nodeCount = data->nodeCount;
    keys = this->keys;
    rots = data->rots;
    unk8s = data->unk8s;
    unkEs = data->unkEs;
    keys += nodeCount * frame;
    for (i = 0; i < nodeCount; i++) {
        node = this->nodes[i];
        if (node != NULL) {
            key = *keys++;
            rot = data->rotMask & key;
            key >>= data->unk8Shift;
            unk8 = data->unk8Mask & key;
            key >>= data->unkEShift;
            unkE = data->unkEMask & key;
            node->to.rot = rots[rot];
            node->to.unk8 = unk8s[unk8];
            node->to.unkE = unkEs[unkE];
            node->from = node->pose;
        }
    }
    return frame - blendFrames;
}

/* Steps the animation by a frame. */
void animatorUpdate(Animator *this) {
    s32 frame;
    s32 again;

    this->flags &= ~ANIMATOR_ENDED;
    if (this->flags & ANIMATOR_STOPPED) {
        return;
    }
    frame = this->frame;
    if (frame < this->blendEnd) {
        animatorSetBlendPoses(this, frame);
    } else {
        animatorSetFramePoses(this, frame);
    }
    animatorUpdateNodes(this);
    frame++;
    if (frame >= this->frameCount) {
        again = this->loops != 0;
        if (this->loops > 0) {
            this->loops--;
        }
        if (again) {
            frame = animatorStartBlend(this, this->loopFrame, 0);
        } else {
            frame = this->frameCount - 1;
            this->flags |= ANIMATOR_STOPPED;
        }
        this->flags |= ANIMATOR_ENDED;
    }
    this->frame = frame;
}

/* Lets every node take on its new pose. */
void animatorUpdateNodes(Animator *this) {
    s32 nodeCount = this->data->nodeCount;
    AnimNode *node;
    s32 i;

    for (i = 0; i < nodeCount; i++) {
        node = this->nodes[i];
        if (node != NULL) {
            node->vtable->update.func((u8 *)node + node->vtable->update.delta);
        }
    }
}

/* Puts every node in its pose of frame. */
void animatorSetFramePoses(Animator *this, s32 frame) {
    AnimData *data = this->data;
    s32 nodeCount = data->nodeCount;
    u32 *keys = this->keys;
    Quaternion *rots = data->rots;
    Vec3s *unk8s = data->unk8s;
    Vec3s *unkEs = data->unkEs;
    AnimNode *node;
    s32 i;
    u32 key;
    u32 rot;
    u32 unk8;
    u32 unkE;

    keys += nodeCount * frame;
    for (i = 0; i < nodeCount; i++) {
        node = this->nodes[i];
        if (node != NULL) {
            key = *keys++;
            rot = data->rotMask & key;
            key >>= data->unk8Shift;
            unk8 = data->unk8Mask & key;
            key >>= data->unkEShift;
            unkE = data->unkEMask & key;
            node->pose.rot = rots[rot];
            node->pose.unk8 = unk8s[unk8];
            node->pose.unkE = unkEs[unkE];
        }
    }
}

/* Puts every node where frame is in the blend. */
void animatorSetBlendPoses(Animator *this, s32 frame) {
    s32 nodeCount;
    AnimNode *node;
    s32 t;
    s32 i;

    /* i counts frames into the blend first */
    i = frame - (this->blendEnd - this->blendFrames);
    t = ((i + 1) << 12) / (this->blendFrames + 1);
    nodeCount = this->data->nodeCount;
    for (i = 0; i < nodeCount; i++) {
        node = this->nodes[i];
        if (node != NULL) {
            animNodeBlend(node, t);
        }
    }
}

/* Blends the pose of node from from to to by t (4.12). */
void animNodeBlend(AnimNode *node, s32 t) {
    blendQuaternion(&node->pose.rot, &node->from.rot, &node->to.rot, t);
    blendVec3s(&node->pose.unk8, &node->from.unk8, &node->to.unk8, t);
    blendVec3s(&node->pose.unkE, &node->from.unkE, &node->to.unkE, t);
}

/* out = (to * t + from * (ONE - t)) >> 12, through the GTE's GPF and GPL.
 * Compiled C (lhu of halfword operands, filled jr delay slot): a C draft over
 * GTE macros has the same instructions, but the original keeps v1 free while
 * loading to and v0 while reading the result, so the registers differ; the
 * cause was not found. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/animator", blendVec3s);

/* Blends from to to by t (4.12). */
void blendQuaternion(Quaternion *out, Quaternion *from, Quaternion *to, s32 t) {
    quaternionSlerp(out, from, to, t);
}
