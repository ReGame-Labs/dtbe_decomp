#include "common.h"
#include "engine/system/break_stub.h"

/* Stops on a break instruction forever, which C cannot express. */
INCLUDE_ASM("asm/jp/main/nonmatchings/system/break_stub", breakForever);
