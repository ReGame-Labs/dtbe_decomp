#include "common.h"
#include <abs.h>
#include <memory.h>
#include "battle/game_classes.h"
#include "engine/task/entity.h"
#include "battle/game.h"
#include "engine/game/game_state.h"
#include "engine/game/loading_screen.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_f3.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/game/character.h"
#include "engine/gfx/sprite.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/lib/list.h"
#include "engine/math/matrix.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/task/task.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "engine/system/memory.h"
#include "engine/text/console.h"
#include "memory.h"
#include "gte.h"
#include "psyq.h"

/* the destructor of an entity class that others derive from */
void func_800776D0(Entity *this, s32 flags);

/* returns the object at 0x228 of `this`: the one whose entity group func_8007AD4C uses */
Unk8006ED20 *func_80077580(Unk80076F34 *this);

/* Entities with a second base at 0x18: the virtual tables of that base move
 * `this` back by 0x18 to reach them. */
typedef struct Unk800ACD74 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
} Unk800ACD74;

/* writes the vector at 0x384 of an Unk80076F34, each component shifted down by 8 bits */
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out);

/* functions of Unk8007F920 that constructors of its kin call */
void func_8007FCFC(Unk8007F920 *this);
void func_8007FED8(Unk8007F920 *this);
void func_8007FF1C(Unk8007F920 *this);

/* returns the word at 0x440 of an Unk80076F34 */
s32 func_8007768C(Unk80076F34 *this);

/* gives an Unk80076F34 a table (at 0x224) */
void func_80079B24(Unk80076F34 *this, Unk80079B24Entry *table);

/* virtual functions of Unk80076F34, and functions of its that they call */
Unk80077698 *func_80077698(Unk80076F34 *this);
s32 func_80077794(Unk80076F34 *this);
void func_8007835C(Unk80076F34 *this, s32 arg);
void func_80078804(Unk80076F34 *this, s32 arg);
void func_8007881C(Unk80076F34 *this, s32 arg);
/* Starts a clip of the model at 0x3FC (modelStartClip; a negative clip stops it):
 * plays < 0 takes the count from the character's .loop file (charaGetClipPlays). */
void func_80079830(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame, s32 plays);
/* The same, with the play count of the .loop file; it keeps clip and
 * blendFrames at 0x250/0x24C, and starts the clip only when func_80079A0C allows it. */
void func_800796F0(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame);
void func_80079AE4(Unk80076F34 *this);
void func_8007A6FC(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3);
void func_8007B034(Unk80076F34 *this, s32 arg);
void func_8007B0E0(Unk80076F34 *this, s32 arg);
void func_8007B470(Unk80076F34 *this, s32 arg, Unk8007B470Entry *entry, s32 arg3);
void func_8007B6A0(Unk80076F34 *this, s32 arg);
void func_8007B70C(Unk80076F34 *this, s32 arg); /* the method at 0xA8 */
void func_8007B784(Unk80076F34 *this, s32 arg); /* the method at 0xB0 */
void func_8007B80C(Unk80076F34 *this, s32 arg); /* the method at 0xB8 */
void func_8007B884(Unk80076F34 *this, s32 arg);
void func_8007BAFC(Unk80076F34 *this, s32 arg);
void func_8007BB64(Unk80076F34 *this, s32 arg);
void func_800A56CC(Unk80076F34 *this, s32 arg);
void func_800BB2B8(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_800BF0E8(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_800BF238(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3);
s32 func_800BF3B8(Unk80076F34 *this);
void func_8007BA98(Unk80076F34 *this, s32 arg);
s32 func_80079E10(Unk80076F34 *this); /* the way the character faces: 1 or -1 */
void func_8007B978(Unk80076F34 *this, s32 arg);
void func_8007B9FC(Unk80076F34 *this, s32 step);
void func_800B6D7C(Unk80076F34 *this);

/* returns the Unk80076F34 that func_80073218 gives for the object at 0x228 of `this` */
Unk80076F34 *func_8007AEEC(Unk80076F34 *this);
void func_80099D18(Unk80076F34 *this);

/* whether the character's fighter (by func_80077574) has the other colors (altColor) */
s32 func_800BF37C(Unk80076F34 *this);

/* returns whether the word that the pointer at 0x230 of the Unk80076F34 of an Unk8007F920 points to is 1 */
s32 func_8007F98C(Unk8007F920 *this);
void func_80080BF0(Unk8007F920 *this, s32 arg);
void func_80081A54(Unk8007F920 *this, s32 arg);

/* gives the world position (in out) of node `node` of the character's model, moved by offset: it hands
 * both to matrixTransformVec */
VECTOR *func_8007A4A4(Unk80076F34 *this, VECTOR *out, s32 node, VECTOR *offset);

/* the gather of the class Unk800C3CFC constructs */
void func_800C3E64(Unk800C3CFC *this);

/* play a sound of an Unk80076F34 and return its handle (the first also keeps it at 0x25C): they call
 * soundControlPlaySound and leave $v0 alone after the call, and func_800BFF28 keeps its next constant out
 * of $v0 (a call_value) */
s32 func_800784F4(Unk80076F34 *this, s32 sound);
s32 func_80078540(Unk80076F34 *this, s32 sound);

/* adds delta to the 16.16 level of the object at 0x238 of an Unk80076F34, kept between 0 and 1.0 */
void func_80077448(Unk80076F34 *this, s32 delta);

/* the constructor of the sprite member of Unk800C2F14, and the setup of its motion */
Unk800C2CCC *func_800C2CCC(Unk800C2CCC *this, u16 *frame, s32 flags);
void func_800C3078(Unk800C2F14 *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color);
void func_800C3100(Unk800C2F14 *this, PolyFT4Shape *shape, SVECTOR *corners);

/* the functions the overrides of entry 0xDC call */
void func_800B1518(Unk80076F34 *this, s32 arg);
void func_800B9F24(Unk80076F34 *this, s32 arg);

/* the parts classes: their constructors, and the vector setter of Unk8007BD8C */
Unk8007BD8C *func_8007BD8C(Unk8007BD8C *this, Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC);
void func_8007BDD4(Unk8007BD8C *this, VECTOR *vec);

/* the side of the character (0x21C): func_800BF37C picks GAME_STATE's fighter with it */
s32 func_80077574(Unk80076F34 *this);

/* the end of an Unk800AC470: spawns two entities and deletes it */
void func_800AC674(Unk800AC470 *this);

/* the parts of entity updates that the functions below call in a row */
void func_8009AFC4(Entity *this, s32 arg);
void func_8009B370(Entity *this, s32 arg);
void func_8009B664(Entity *this, s32 arg);
void func_800A7290(Entity *this, s32 arg);
void func_800A7458(Entity *this, s32 arg);
void func_800A8C24(Entity *this, s32 arg);
void func_800A8F80(Entity *this, s32 arg);
void func_800A9998(Entity *this, s32 arg);
void func_800BE7D0(Entity *this, s32 arg);
void func_800BEA1C(Entity *this, s32 arg);

/* the constructors the functions below call */
Unk80076F34 *func_80076F34(Unk80076F34 *this);
Unk8007F920 *func_8007F920(Unk8007F920 *this, s32 unk0);
Unk8007F920 *func_800808EC(Unk8007F920 *this, s32 arg);
Entity *func_80082EE8(Entity *this, Unk80076F34 *arg, Vec3 *vel, s32 arg3, s32 side);
struct Unk80083700 *func_80083700(struct Unk80083700 *this, Unk80076F34 *chara);
Entity *func_8008514C(Entity *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Entity *func_80085B7C(Entity *this, Unk80076F34 *arg, Vec3 *vec);
Entity *func_800862C0(Entity *this, Unk80076F34 *arg, Vec3 *vec);
struct Unk8008A9BC *func_8008A9BC(struct Unk8008A9BC *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                                  Unk8006ED20 *list);
Entity *func_800856D0(Entity *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj);
struct Unk8008B124 *func_8008B124(struct Unk8008B124 *this, Unk80076F34 *chara);
struct Unk800833F8 *func_800833F8(struct Unk800833F8 *this, Vec3 *position, s32 arg2, s32 arg3, s32 vramY);
struct Unk800971E0 *func_800971E0(struct Unk800971E0 *this, Unk80076F34 *chara, s32 arg2, Unk8006ED20 *obj);
struct Unk800B8FAC *func_800B8FAC(struct Unk800B8FAC *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj);
Entity *func_800C82CC(Entity *this, Unk8006ED20 *owner, void *arg2, s32 arg3);
struct Unk80093AD8 *func_80093AD8(struct Unk80093AD8 *this, Unk80076F34 *chara, Unk8006ED20 *obj);
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3);
Entity *func_800B5920(Entity *this, Unk80076F34 *chara, Unk8006ED20 *obj, VECTOR *position);
void func_800C72D8(Unk800C7024 *this, Unk80075EE4 *sum);
Entity *func_800868EC(Entity *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity, Unk8006ED20 *obj);
Entity *func_8009764C(Entity *this, Unk80076F34 *chara, Unk8006ED20 *obj, s32 side);
Entity *func_8009AB40(Entity *this, Unk80076F34 *chara, s32 vramY);
Entity *func_800925AC(Entity *this, Unk80076F34 *chara, Vec3 *position);
Entity *func_80093460(Entity *this, Unk80076F34 *chara, Unk8006ED20 *obj, Vec3 *position, Vec3 *velocity);
Entity *func_800B50EC(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *obj);
struct Unk8008D21C *func_8008D21C(struct Unk8008D21C *this);
Entity *func_8008F474(Entity *this, Unk80076F34 *arg, VECTOR *pos, Unk8006ED20 *arg3);
Entity *func_800962B8(Entity *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos, VECTOR *vel);
Entity *func_80096F6C(Entity *this, Unk80076F34 *arg);
Entity *func_80097FA0(Entity *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Entity *func_80098720(Entity *this, Unk80076F34 *arg, Vec3 *vec);
Entity *func_8009E8F8(Entity *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos, VECTOR *vel);
Entity *func_8009F76C(Entity *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, s32 side, s32 arg5);
struct Unk800A0054 *func_800A0054(struct Unk800A0054 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos,
                                  Vec3 *vel, s32 arg5);
struct Unk800A0830 *func_800A0830(struct Unk800A0830 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos,
                                  Vec3 *vel, s32 arg5);
Unk8009F248 *func_8009F248(Unk8009F248 *this, Unk80076F34 *owner);
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Unk80076F34 *owner);
Entity *func_800A47F4(Entity *this, Unk80076F34 *arg);
struct Unk800A6220 *func_800A6220(struct Unk800A6220 *this, Unk80076F34 *chara);
Entity *func_800AD5BC(Entity *this, Unk80076F34 *arg1, Unk8006ED20 *arg2, Vec3 *pos, s32 arg4, s32 arg5);
Unk800AED0C *func_800AED0C(Unk800AED0C *this, Unk80076F34 *arg);
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Unk800B050C *func_800B050C(Unk800B050C *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Entity *func_800B6DD8(Entity *this, Unk800B631C *owner, VECTOR *pos, Unk8006ED20 *arg3, u32 color);
Unk800B73E8 *func_800B73E8(Unk800B73E8 *this, Unk80076F34 *arg);
Unk800C7024 *func_800B719C(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity,
                          Unk8006ED20 *obj);
Entity *func_800B7820(Entity *this, Unk80076F34 *arg);
struct Unk800B8BBC *func_800B8BBC(struct Unk800B8BBC *this, Unk80076F34 *chara, VECTOR *pos, Unk8006ED20 *obj);
Entity *func_800BD490(Entity *this, Unk80076F34 *arg);
Entity *func_800BDA50(Entity *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4);
Entity *func_800C06D0(Entity *this, Unk80076F34 *arg);
Entity *func_800C0CC0(Entity *this, Unk80076F34 *arg, Vec3 *vec);
Unk800C2F14 *func_800C2FBC(Unk800C2F14 *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags);
Entity *func_800C3654(Entity *this, Vec3 *unk28, Vec3 *unk40, s32 unk20, u32 color, s32 unk18);
Unk800C3CFC *func_800C3CFC(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3);
/* An entry of the tables func_800C5680 takes (func_800C5B18 copies up to 4 of them, all but unk10). */
typedef struct Unk800C5B18Entry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ Vec3 unk4;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ CVECTOR color;
} Unk800C5B18Entry;

Entity *func_800C5680(Entity *this, Unk80076F34 *arg, Unk800C5B18Entry *entries, s32 count, s32 arg4, s32 arg5);
/* the constructors of Unk800C6088's two classes: count particles around pos; arg6 is a color (0x808080) */
Unk800C6088 *func_800C6088(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6);
Unk800C6088 *func_800C62EC(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6);
Entity *func_800C8778(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6);
Entity *func_800CA430(Entity *this, Unk80076F34 *arg, s32 arg2);
Entity *func_800CA768(Entity *this, Unk80076F34 *arg, s32 arg2);
Entity *func_800CAACC(Entity *this, Unk80076F34 *chara, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/* the virtual tables the functions below install, and the data they read */
extern VtableEntry D_80067E0C[];
extern VtableEntry D_80067E7C[];
extern VtableEntry D_80067E44[];
extern VtableEntry D_800652CC[];
extern VtableEntry D_80068154[];
extern VtableEntry D_80068004[];
extern VtableEntry D_80068074[];
extern VtableEntry D_800680E4[];
extern VtableEntry D_8006811C[];
extern Unk80079B24Entry D_800681DC[];
extern EntityVtable D_8006843C;
extern VtableEntry D_8006869C[];
extern EntityVtable D_800686BC;
extern EntityVtable D_800686E4;
extern EntityVtable D_8006870C;
extern EntityVtable D_80068734;
extern EntityVtable D_8006875C;
extern EntityVtable D_80068784;
extern Unk80079B24Entry D_8006894C[];
extern EntityVtable D_800689F4;
extern Unk80079B24Entry D_80068B6C[];
extern VtableEntry D_80068BAC[];
extern EntityVtable D_80068BCC;
extern VtableEntry D_80068BF4[];
extern EntityVtable D_80068C14;
extern EntityVtable D_80068C3C;
extern EntityVtable D_80068D84;
extern EntityVtable D_80068DAC;
extern EntityVtable D_80068DD4;
extern EntityVtable D_80068DFC;
extern VtableEntry D_80068E24[];
extern EntityVtable D_80068E44;
extern EntityVtable D_80068E6C;
extern VtableEntry D_80068E94[];
extern EntityVtable D_80068EB4;
extern EntityVtable D_80068EDC;
extern VECTOR D_800690E0;
extern Unk800C5B18Entry D_800690F0[];
extern EntityVtable D_800691EC;
extern VtableEntry D_80069214[];
extern EntityVtable D_80069234;
extern EntityVtable D_8006925C;
extern VtableEntry D_80069284[];
extern EntityVtable D_800692A4;
extern EntityVtable D_800692CC;
extern VtableEntry D_800692F4[];
extern EntityVtable D_80069314;
extern EntityVtable D_8006933C;
extern EntityVtable D_80069364;
extern EntityVtable D_8006938C;
extern EntityVtable D_8006D574;
extern Unk800C5B18Entry D_800694B4[];
extern EntityVtable D_800694F4;
extern VtableEntry D_8006951C[];
extern EntityVtable D_8006953C;
extern EntityVtable D_80069564;
extern EntityVtable D_8006972C;
extern EntityVtable D_80069754;
extern EntityVtable D_8006977C;
extern EntityVtable D_800697A4;
extern EntityVtable D_8006983C;
extern VECTOR D_80069964;
extern EntityVtable D_800699DC;
extern EntityVtable D_80069A04;
extern EntityVtable D_80069A2C;
extern EntityVtable D_80069A54;
extern EntityVtable D_80069A7C;
extern EntityVtable D_80069ACC;
extern EntityVtable D_80069AF4;
extern EntityVtable D_80069B6C;
extern EntityVtable D_80069BDC;
extern EntityVtable D_80069C04;
extern EntityVtable D_8006A15C;
extern VtableEntry D_8006A1CC[];
extern EntityVtable D_8006A1EC;
extern EntityVtable D_8006A2AC;
extern VECTOR D_8006A43C;
extern VECTOR D_8006A44C[2];
extern VECTOR D_8006A46C[2];
extern EntityVtable D_8006A48C;
extern EntityVtable D_8006A4B4;
extern EntityVtable D_8006A4DC;
extern EntityVtable D_8006A504;
extern EntityVtable D_8006A52C;
extern EntityVtable D_8006A554;
extern EntityVtable D_8006A5C4;
extern u16 D_8006A6EC[];
extern u16 D_8006A700[];
extern Unk80079B24Entry D_8006A764[];
extern EntityVtable D_8006A784;
extern EntityVtable D_8006A7AC;
extern EntityVtable D_8006A7FC;
extern VtableEntry D_8006AA1C[];
extern EntityVtable D_8006AA3C;
extern VtableEntry D_8006AF2C[];
extern EntityVtable D_8006AF4C;
extern VtableEntry D_8006AFC4[];
extern EntityVtable D_8006AFE4;
extern EntityVtable D_8006B00C;
extern EntityVtable D_8006B03C;
extern VtableEntry D_8006B314[];
extern EntityVtable D_8006B334;
extern VtableEntry D_8006B35C[];
extern EntityVtable D_8006B37C;
extern VtableEntry D_8006B3A4[];
extern EntityVtable D_8006B3C4;
extern VtableEntry D_8006B3EC[];
extern EntityVtable D_8006B40C;
extern VtableEntry D_8006B434[];
extern EntityVtable D_8006B454;
extern VtableEntry D_8006B47C[];
extern EntityVtable D_8006B49C;
extern EntityVtable D_8006B4C4;
extern EntityVtable D_8006B5FC;
extern EntityVtable D_8006B624;
extern EntityVtable D_8006B64C;
extern VtableEntry D_8006B674[];
extern EntityVtable D_8006B694;
extern EntityVtable D_8006B6BC;
extern EntityVtable D_8006B6E4;
extern VtableEntry D_8006B70C[];
extern EntityVtable D_8006B72C;
extern EntityVtable D_8006B754;
extern VtableEntry D_8006BAE4[];
extern EntityVtable D_8006BB04;
extern EntityVtable D_8006BB2C;
extern EntityVtable D_8006BBE4;
extern EntityVtable D_8006BC0C;
extern VtableEntry D_8006BC34[];
extern EntityVtable D_8006BC54;
extern EntityVtable D_8006BCA4;
extern Unk80079B24Entry D_8006BE1C[];
extern EntityVtable D_8006BE4C;
extern EntityVtable D_8006BE74;
extern EntityVtable D_8006BEE4;
extern Unk80079B24Entry D_8006C05C[];
extern EntityVtable D_8006C07C;
extern EntityVtable D_8006C0A4;
extern VtableEntry D_8006C114[];
extern EntityVtable D_8006C134;
extern EntityVtable D_8006C15C;
extern Unk80079B24Entry D_8006C324[];
extern EntityVtable D_8006C334;
extern EntityVtable D_8006C35C;
extern EntityVtable D_8006C384;
extern Unk80079B24Entry D_8006C4FC[];
extern VtableEntry D_8006C6AC[];
extern EntityVtable D_8006C6CC;
extern EntityVtable D_8006C71C;
extern VtableEntry D_8006C9DC[];
extern EntityVtable D_8006C9FC;
extern VtableEntry D_8006CAD4[];
extern EntityVtable D_8006CAF4;
extern EntityVtable D_8006CDD4;
extern EntityVtable D_8006CDFC;
extern EntityVtable D_8006CE4C;
extern s32 D_8006CF74[];
extern s32 D_8006CF7C[];
extern EntityVtable D_8006D01C;
extern EntityVtable D_8006D044;
extern EntityVtable D_8006D29C;
extern EntityVtable D_8006D2C4;
extern EntityVtable D_8006D2EC;
extern EntityVtable D_8006D45C;
extern EntityVtable D_8006D484;
extern EntityVtable D_8006D4AC;
extern EntityVtable D_8006D4FC;
extern EntityVtable D_8006D59C;
extern VtableEntry D_8006D740[];
extern EntityVtable D_8006D760;
extern EntityVtable D_8006D9F0;
extern EntityVtable D_8006DA18;
extern EntityVtable D_8006DA40;
extern EntityVtable D_8006DA68;
extern EntityVtable D_8006DAC0;
extern EntityVtable D_8006DAF0;
extern EntityVtable D_8006DA90;
extern EntityVtable D_8006DB20;
extern VtableEntry D_8006DB80[];
extern EntityVtable D_8006DBA0;
extern LoadingScreenVtable D_8006DF3C;
extern EntityVtable D_8006E014;
extern EntityVtable D_8006E03C;
extern EntityVtable D_8006E064;
extern EntityVtable D_8006E08C;
extern EntityVtable D_8006E0B4;
extern EntityVtable D_8006E0DC;
extern EntityVtable D_8006E104;
extern EntityVtable D_8006E12C;
extern VtableEntry D_8006E154[];
extern VECTOR D_800D348C;
extern u8 D_8006DFCC[];
extern u8 D_8006DFD4[]; /* "%03d" */

/* A class derived from Unk8007F920 (func_8007F6FC), not identified yet. */
typedef struct Unk8007F6FC {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ u8 unk10[0x30 - 0x10];
    /* 0x30 */ s32 unk30;
} Unk8007F6FC;

void func_8007F6FC(Unk8007F6FC *this);

/* The task of virtual table D_80064434: its update stores the argument at
 * 0x20 and calls the task's own first virtual method (at 0x20). title.bin has
 * six tasks with the same update (Unk8006713C there). */
typedef struct Unk8006ECECVtable {
    /* 0x00 */ TaskVtable task;
    /* 0x20 */ VtableEntry unk20;
} Unk8006ECECVtable;

typedef struct Unk8006ECEC {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ Unk8006ECECVtable *vtable;
    /* 0x20 */ s32 unk20; /* the argument of the last update */
} Unk8006ECEC;

/* A g++ 2.95 pointer to member function, laid out as a VtableEntry: index
 * -1 for a non-virtual function, which is then func. The game keeps such
 * constants in .rodata (D_80067C54 and the ones around it, each
 * {0xFFFF0000, function}). */
typedef VtableEntry MemberFunction;

/* The virtual table of Unk80076F34 (D_8006530C): Entity's four entries, then
 * the class's own methods, which its derived classes override. */
typedef struct Unk80076F34Vtable {
    /* 0x000 */ EntityVtable entity;
    /* 0x020 */ VtableEntry unk20;
    /* 0x028 */ VtableEntry unk28;
    /* 0x030 */ VtableEntry unk30;
    /* 0x038 */ VtableEntry unk38;
    /* 0x040 */ VtableEntry unk40;
    /* 0x048 */ VtableEntry unk48;
    /* 0x050 */ VtableEntry unk50;
    /* 0x058 */ VtableEntry unk58;
    /* 0x060 */ VtableEntry unk60;
    /* 0x068 */ VtableEntry unk68;
    /* 0x070 */ VtableEntry unk70;
    /* 0x078 */ VtableEntry unk78;
    /* 0x080 */ VtableEntry unk80;
    /* 0x088 */ VtableEntry unk88;
    /* 0x090 */ VtableEntry unk90;
    /* 0x098 */ VtableEntry unk98;
    /* 0x0A0 */ VtableEntry unkA0;
    /* 0x0A8 */ VtableEntry unkA8;
    /* 0x0B0 */ VtableEntry unkB0;
    /* 0x0B8 */ VtableEntry unkB8;
    /* 0x0C0 */ VtableEntry unkC0;
    /* 0x0C8 */ VtableEntry unkC8;
    /* 0x0D0 */ VtableEntry unkD0;
    /* 0x0D8 */ VtableEntry unkD8;
    /* 0x0E0 */ VtableEntry unkE0;
    /* 0x0E8 */ VtableEntry unkE8;
    /* 0x0F0 */ VtableEntry unkF0;
    /* 0x0F8 */ VtableEntry unkF8;
    /* 0x100 */ VtableEntry unk100;
    /* 0x108 */ VtableEntry unk108;
    /* 0x110 */ VtableEntry unk110;
    /* 0x118 */ VtableEntry unk118;
    /* 0x120 */ VtableEntry unk120;
    /* 0x128 */ VtableEntry unk128;
    /* 0x130 */ VtableEntry unk130;
    /* 0x138 */ VtableEntry unk138;
} Unk80076F34Vtable;

/* An Unk80076F34's virtual table as its own class sees it. */
#define UNK80076F34_VTABLE(object) ((Unk80076F34Vtable *)(object)->entity.vtable)

typedef struct Unk80083F0C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ s16 unk4A6;
} Unk80083F0C;

typedef struct Unk80089FB8 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ s16 unk4A6;
    /* 0x4A8 */ s16 unk4A8;
} Unk80089FB8;

typedef struct Unk80094F10 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
} Unk80094F10;

typedef struct Unk80099A7C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ Unk80076F34 *unk4A8;
    /* 0x4AC */ u8 unk4AC[0x4CC - 0x4AC];
    /* 0x4CC */ u32 unk4CC; /* a sound's handle (stopSound) */
} Unk80099A7C;

/* The character class of vtable D_8006A5C4 (func_8009DBBC): func_8009E744 and func_8009E814 clear
 * 0x4A8 and 0x4AC before they launch their shots. */
typedef struct Unk8009DBBC {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
} Unk8009DBBC;

typedef struct Unk800AA390 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
} Unk800AA390;

typedef struct Unk800ADA44 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ s32 unk4A8;
} Unk800ADA44;

typedef struct Unk800B0854 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
} Unk800B0854;

/* derived from func_80076F34's class, and the base of func_800C1924's */
typedef struct Unk800BB034 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
} Unk800BB034;

/* The class func_800A54C8 constructs (virtual table D_8006B06C), derived
 * from func_80076F34's: func_800A58F4 keeps four results of func_800A5B98 at
 * 0x4A8-0x4B4. */
typedef struct Unk800A54C8 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
} Unk800A54C8;

extern EntityVtable D_8006B06C;

typedef struct Unk800C1924 {
    /* 0x000 */ Unk800BB034 base;
    /* 0x4C4 */ s32 unk4C4;
} Unk800C1924;

extern EntityVtable D_8006CB44;

Unk800BB034 *func_800BB034(Unk800BB034 *this);

/* The entity func_800C49E4 constructs (virtual table D_8006D394) for a
 * character's state object, at the character's position. */
typedef struct Unk800C49E4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007F920 *unk18;
    /* 0x1C */ Vec3 position; /* the character's (func_800775D8) */
} Unk800C49E4;

/* constructs the entity of virtual table D_8006A25C for a character */
Entity *func_8009B834(void *this, Unk80076F34 *chara, s32 arg2, s32 arg3);

/* The entity class of virtual table D_8006A15C, which func_8009A814 makes for its object. */
typedef struct Unk8009D20C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ void *unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} Unk8009D20C;

/* The entity classes that func_800D0CF8 and func_800D0F1C make for
 * func_8006ED20's object: D_8006E12C, D_8006E104, D_8006E0DC, and D_8006E0B4 with a
 * field more. */
typedef struct Unk800D1328 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *unk18;
} Unk800D1328;
typedef struct Unk800D1C50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *unk18;
    /* 0x1C */ s16 unk1C;
} Unk800D1C50;

typedef struct Unk80080C5C {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk80080C5C;

/* The class of virtual table D_80067E44 (func_80081874): func_8007F920's with a word at 0x10. */
typedef struct Unk80081874 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk80081874;

Unk8007F920 *func_80080778(Unk8007F920 *this, s32 unk0);
/* func_800796F0 on the character at 8, from frame 0 */
void func_8007FC2C(Unk8007F920 *this, s32 clip, s32 blendFrames);

/* The object at 4 of Unk800C8778: its virtual table, at 0x14 as an Entity's,
 * has a method at 0x20 that func_800C8DFC calls. */
typedef struct Unk800C8DFCVtable {
    /* 0x00 */ VtableEntry unk0[4];
    /* 0x20 */ VtableEntry unk20;
} Unk800C8DFCVtable;

typedef struct Unk800C8DFC {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Unk800C8DFCVtable *vtable;
} Unk800C8DFC;

/* The entity func_8008F474 constructs (virtual table D_800692CC): ten points
 * scattered around a centre, each with an angle. */
typedef struct Unk8008F474 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x38 - 0x18];
    /* 0x38 */ Vec3 unk38; /* the centre, func_8008F474's third argument */
    /* 0x44 */ s32 unk44;
    /* 0x48 */ VECTOR unk48[10];
    /* 0xE8 */ s32 unkE8[10];
} Unk8008F474;

/* The entity func_800B2E68 constructs (virtual table D_8006BB74, D_8006BB54
 * for its Unk8007BD8C at 0x18): sixteen points around a centre. */
typedef struct Unk800B2E68 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 part;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0x8A - 0x84];
    /* 0x08A */ u16 unk8A; /* while set, the destructor calls func_80079274 on func_8007AEEC's character */
    /* 0x08C */ u8 unk8C[0xAC - 0x8C];
    /* 0x0AC */ Vec3 unkAC;
    /* 0x0B8 */ u8 unkB8[4];
    /* 0x0BC */ Vec3 unkBC; /* func_800B2E68's third argument */
    /* 0x0C8 */ u8 unkC8[4];
    /* 0x0CC */ VECTOR unkCC[16];
} Unk800B2E68;

/* The entity func_800C4204 constructs (virtual table D_8006D3E4), with a Sprite at 0x28. */
typedef struct Unk800C4204 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x28 - 0x18];
    /* 0x28 */ Sprite sprite;
} Unk800C4204;

extern EntityVtable D_8006DD6C;
/* adds the three words at 0x28 to 0x34 and clears them (the D_8006DD6C entity's) */
void func_800CDFBC(Entity *this);

/* The entity class func_800C8778 constructs (virtual table D_8006DB50; the
 * classes of D_8006DAC0, D_8006DAF0 and D_8006DB20 derive from it). */
/* The class func_800C8778 constructs (virtual table D_8006DB80), derived from Unk80076308. */
typedef struct Unk800C8778 {
    /* 0x0 */ Unk80076308 base;
    /* 0x4 */ Unk800C8DFC *unk4;
} Unk800C8778;

typedef struct Unk800C90FC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80075EE4 part;
    /* 0x60 */ Unk800C8778 unk60;
    /* 0x68 */ s32 unk68; /* func_800C8778's fifth argument */
} Unk800C90FC;

void func_800C9140(Unk800C90FC *this, s32 arg);
void func_800C9288(Unk800C90FC *this, s32 arg);

/* The classes func_800AD5BC and func_800893BC construct (virtual tables
 * D_8006B314 and D_80068BAC for their base at 0x18); the methods of
 * D_8006BC34 and D_8006BAE4 (func_800B1FD4, func_800B427C) see the same part. */
typedef struct Unk800AD5BC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x44 - 0x34];
    /* 0x44 */ u32 unk44;
} Unk800AD5BC;

/* The classes of D_8006AF2C and D_8006AE8C (virtual tables of their base at
 * 0x18), whose methods func_800A7F88 and func_800A9944 are. */
typedef struct Unk800A7F88 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x44 - 0x34];
    /* 0x44 */ u32 unk44;
    /* 0x48 */ u8 unk48[0xAC - 0x48];
    /* 0xAC */ s32 unkAC;
} Unk800A7F88;

typedef struct Unk800A9944 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ u8 unk80[0xB4 - 0x80];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ u8 unkB8[0xD4 - 0xB8];
    /* 0xD4 */ u32 unkD4; /* a sound's handle (stopSound) */
} Unk800A9944;

/* The class func_800B50EC constructs (virtual tables D_8006BEBC, and D_8006BE9C for its base at
 * 0x18). */
typedef struct Unk800B50EC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0xC8 - 0x84];
    /* 0x0C8 */ Vec3 unkC8;
    /* 0x0D4 */ u8 unkD4[0x118 - 0xD4];
    /* 0x118 */ Vec3 position;
    /* 0x124 */ u8 unk124[0x128 - 0x124];
    /* 0x128 */ Vec3 velocity;
    /* 0x134 */ u8 unk134[0x158 - 0x134];
    /* 0x158 */ s32 speed; /* the length of velocity */
    /* 0x15C */ u32 unk15C; /* a sound's handle: func_80078540's (sound 0xF2), stopSound */
} Unk800B50EC;

extern VtableEntry D_8006AE8C[];
extern EntityVtable D_8006AEAC;
extern VtableEntry D_8006BE9C[];
extern EntityVtable D_8006BEBC;

/* The class of func_800A4550 and func_800A45C8, which spawn entities at its
 * two positions. */
typedef struct Unk800A4550 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x84 - 0x18];
    /* 0x84 */ Vec3 unk84;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ Vec3 unkA4;
} Unk800A4550;

/* The entity class func_8008787C constructs (virtual table D_80068984) for a
 * character. */
typedef struct Unk8008787C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50; /* the character's 0x440 (func_8007768C) */
} Unk8008787C;

Unk8008787C *func_8008787C(Unk8008787C *this, Unk80076F34 *chara);

extern EntityVtable D_80068984;

/* The entity classes func_80093AD8 and func_800A8858 construct (virtual
 * tables D_8006977C and D_8006AEDC) for a character. */
typedef struct Unk80093AD8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C[9];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ u8 unk44[4];
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s16 unk54; /* the character's 0x440 (func_8007768C) */
} Unk80093AD8;

typedef struct Unk800A8858 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C[11];
    /* 0x48 */ s32 unk48; /* the character's 0x440 (func_8007768C) */
} Unk800A8858;

extern EntityVtable D_8006AEDC;

/* The class of func_80083108, derived from Unk800C7024 */
typedef struct Unk80083108 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ s32 unkC0;
} Unk80083108;

/* func_800C7024 constructs an entity for chara at position, moving by
 * velocity; the other four arguments are set by the derived classes. */
Unk800C7024 *func_800C7024(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, s32 unkA8, u32 unk14,
                                s32 unkB4, s32 unk1C);

extern EntityVtable D_800683EC;
extern EntityVtable D_800689CC;
extern EntityVtable D_8006C0EC;
extern VtableEntry D_800683CC[];
extern VtableEntry D_800689AC[];
extern VtableEntry D_8006C0CC[];

/* Returns the entity at 0x14 of arg when func_8002BD68 gives 0x10000 for it, else NULL. */
Entity *func_80078878(void *arg);

/* The entity class func_800BDA50 constructs for a character (virtual table D_8006C9FC; func_800C2264's
 * class, D_8006CFEC, derives from it): Unk8007BD8C at 0x18, a member of func_80075EE4's class at 0x38. */
typedef struct Unk800BDA50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ s32 unk80; /* the character's entityGetHandle */
    /* 0x84 */ u8 unk84[0x94 - 0x84];
    /* 0x94 */ Vec3 unk94; /* func_800BDA50's third argument */
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ Vec3 unkA4; /* 0 from func_800BDA50 */
} Unk800BDA50;

Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4);

/* 4 bytes func_800BE39C copies to 0x94 of its entity */
extern CVECTOR D_8006C964;
extern CVECTOR D_8006CFA8;
Entity *func_800BE39C(Entity *this, s32 arg1, Vec3 *position, u32 arg3, s32 arg4, CVECTOR *arg5);

/* The class func_800BE39C constructs (virtual table D_8006C9AC, and D_8006C98C for its base at 0x18),
 * derived from func_800A926C's. */
typedef struct Unk800BE39C {
    /* 0x00 */ Unk800A9944 base;
} Unk800BE39C;

extern VtableEntry D_8006C98C[];
extern EntityVtable D_8006C9AC;

/* The object of func_800C6EC4. */
typedef struct Unk800C6EC4 {
    /* 0x00 */ u8 unk0[0x24];
    /* 0x24 */ s32 unk24;
} Unk800C6EC4;

/* The member at 0x260 of Unk80076F34, which func_80076444 constructs. */
/* A node of the list whose head is D_8006437C, mesh_scene's global list
 * (func_80076A00 gives it; func_80076A0C moves a node to its front). func_80076474 rotates with the
 * matrix at 0xC of the node it picks. */
typedef struct Unk80076A0C {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ MATRIX *unkC;
} Unk80076A0C;

typedef struct Unk80076444 {
    /* 0x00 */ u8 unk0[0x78];
    /* 0x78 */ Unk80076A0C *unk78; /* the node of D_8006437C func_80076474 last picked */
} Unk80076444;

Unk80076444 *func_80076444(Unk80076444 *this);
void func_80076474(Unk80076444 *this, Vec3 *a, Vec3 *b);

/* two of these are at 0x18 and 0x28 of the entity func_800733C4 constructs,
 * each set up by func_800735E8 with 0x10000 at 0xC */
typedef struct Unk800735E8 {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} Unk800735E8;

typedef struct Unk800763F4 {
    /* 0x00 */ u8 unk0[0x34];
    /* 0x34 */ Vec3 unk34;
} Unk800763F4;

typedef struct Unk800AA04C {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ Vec3 unk20;
} Unk800AA04C;

/* the second argument of func_800C8F04 */
typedef struct Unk800C8F04 {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Entity *unk14;
} Unk800C8F04;

/* The box func_8007691C tests points against: two corners. */
typedef struct Unk8007691C {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
} Unk8007691C;

/* the destructor of Unk800C90FC, the entity class others derive from */
void func_800C8920(Unk800C90FC *this, s32 flags);

/* What func_80075000 constructs, one per character (Unk8006ED20.unk24): the
 * character's pad input, kept by func_8007504C every frame. */
typedef struct Unk80075000 {
    /* 0x000 */ u16 history[256]; /* the buttons held, frame by frame */
    /* 0x200 */ u16 held;
    /* 0x202 */ u16 pressed;  /* held now and not the frame before */
    /* 0x204 */ u16 released; /* held the frame before and not now */
    /* 0x206 */ u8 unk206[2];
    /* 0x208 */ s32 count;    /* the next entry of history, 0 to 255 */
    /* 0x20C */ s32 unk20C;
    /* 0x210 */ s32 unk210;
    /* 0x214 */ u32 unk214;   /* PAD_LEFT | PAD_RIGHT or 0: the buttons func_80075574 swaps */
} Unk80075000;

void func_80075034(void *this, s32 arg);
void func_80075090(void *this);
Vec3 *func_8007A650(Unk80076F34 *chara, Vec3 *out);
/* buttons with the two of 0x214 swapped when only one of them is in it */
u32 func_80075574(Unk80075000 *this, u32 buttons);
/* the last entry of history, through func_80075574 */
u32 func_800750C4(Unk80075000 *this);
void func_8007BDF0(Unk8007BD8C *this, Unk80075EE4 *sum);
void *func_8007C17C(void *memory, s32 arg);
void func_8007FC50(void *this, s32 arg1, s32 arg2);
void func_80078910(void *this, void *arg);
void func_800AB830(UnkPartsEntity38 *this);
void func_800C1CE0(void *this);
void func_8006EF90(Unk8006ED20 *object, s32 index, s32 add);
s32 func_8007691C(Unk8007691C *box, s32 x, s32 y);
ListNode *func_80076A00(void);
void func_8006E330(void *object);
void func_80071110(Unk8006ED20 *object);
void func_80079B2C(Unk80076F34 *this, s32 arg);
void func_80082AF8(Unk80076F34 *this);
void func_80087400(Unk80076F34 *this);
void func_800AE16C(Unk80076F34 *this);
void func_800B6868(Unk80076F34 *this);
void func_800B6910(Unk80076F34 *this);
void func_8008A384(struct Unk80089FB8 *this);
void func_80092F58(Unk80076F34 *this);
void func_8008A44C(Unk80076F34 *this);
Entity *func_80098A78(Entity *this, Unk80076F34 *chara, Unk8006ED20 *unk228, Vec3 *position, Vec3 *velocity);
Entity *func_800A1E70(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
Unk800C7024 *func_80087690(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);
/* constructs an entity (on func_800C2F14's) for chara at position with velocity */
Entity *func_80093274(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);
/* this->unk218 */
s32 func_80077568(Unk80076F34 *this);
/* this->unk334 = arg */
void func_80079B0C(Unk80076F34 *this, s32 arg);
void func_8008E51C(Unk80076F34 *this);
void func_8008E5C4(Unk80076F34 *this, s32 arg);
void func_80079320(Unk80076F34 *this);
void func_800C0084(Unk80076F34 *this);
void func_800C00EC(Unk80076F34 *this);
void func_800C0210(Unk80076F34 *this);
Entity *func_800B9FF0(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
s32 func_8007A044(Unk80076F34 *this, s32 arg);
void func_800C2E84(Unk800C2CCC *sprite, PolyFT4Shape *shape, SVECTOR *corners);
void func_800C5834(Unk800C6EC4 *this);
void func_800C59D4(Unk800C6EC4 *this);
void func_8007AE34(Unk80076F34 *this);
s32 func_8007F5A8(Unk8007F184 *this);
Unk8007F184 *func_8007758C(Unk80076F34 *this);
void func_8007F1B8(Unk8007F184 *this);
void func_80081438(Unk8007F920 *this, s32 arg);
Unk8007F920 *func_800819E0(Unk8007F920 *this);
void func_80079350(Unk80076F34 *this);
void func_800C9790(Unk80076F34 *this);
s32 func_800798F4(Unk80076F34 *this, s32 arg);
s32 func_800778D0(Unk80076F34 *this, s32 arg);
void func_800788B4(Unk80076F34 *this, s32 arg1, s32 arg2);
void func_800C99B0(Unk80076F34 *this);
/* this->unk22C and this->unk230 */
Unk8007F920 *func_800792A0(Unk80076F34 *this);
Unk80075000 *func_800776C4(Unk80076F34 *this); /* returns 0x234 */
void func_8008E900(Unk80076F34 *this);
void func_8007B2C0(Unk80076F34 *this, s32 arg);
void func_8007B2EC(Unk80076F34 *this, s32 arg);
Entity *func_800BAAA4(void *this, Unk80076F34 *chara, s32 arg);
void func_800B19CC(Unk800B0854 *this);
s32 *func_800792AC(Unk80076F34 *this);
/* the constructors of the entities spawned below */
Entity *func_8008EAB4(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
Entity *func_800902E0(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
Entity *func_80091E60(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
/* an offset from a node of the character's model */
extern VECTOR D_80010824; /* a zero offset in the executable's .rodata */
extern VECTOR D_80068654;
extern VECTOR D_800690D0;
extern Unk800C5B18Entry D_8006B1AC[];
extern VECTOR D_800D345C; /* {0, -70, 40, 0x1000} */
void func_8007F0F4(Unk8007F184 *this, s32 amount);
void func_8007F1FC(Unk8007F184 *this, s32 amount);
s32 func_8007FB9C(Unk8007F920 *this);
/* Constructs, in place, the state of virtual table D_80067DD4 (on
 * func_80081A20's) with arg and the step function, which it then calls. */
Unk8007F920 *func_8008209C(Unk8007F920 *this, s32 arg, MemberFunction step);
/* the step functions of func_8008209C's states (pointers to member functions) */
extern MemberFunction D_80067CCC;
extern MemberFunction D_80067CD4;
extern MemberFunction D_80067CDC;
extern MemberFunction D_80067CE4;
extern MemberFunction D_80067CEC;
extern MemberFunction D_80067CF4;
/* Constructs, in place, the state of virtual table D_80067D94 (on
 * func_8007F920's) with arg and the step function at 0x14. */
Unk8007F920 *func_800823EC(Unk8007F920 *this, s32 arg, MemberFunction step);
extern MemberFunction D_80067D04;
extern MemberFunction D_80067D0C;
extern VtableEntry D_80067D14[];
extern VtableEntry D_80067D54[];
void *func_800807F4(Unk8007F920 *this, s32 arg);
void func_800C8DFC(Unk800C8DFC *object, s32 arg);
void func_800C89B4(void *this);
Unk8007F920 *func_80082690(Unk8007F920 *this);
Unk8007F920 *func_800826E8(Unk8007F920 *this);
ListNode *func_80075C64(ListNode *head, ListNode *node);
void func_80093144(void *this);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", gameMainTaskInit);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006E23C);

/* gameMainTaskInit's task (virtual table D_80064434), as its first virtual method sees it: a thread at 0x28. */
typedef struct Unk8006E2EC {
    /* 0x00 */ Unk8006ECEC base;
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ Thread thread;
} Unk8006E2EC;

/* The first virtual method of D_80064434 (at 0x20): switches to the task's thread with 0. */
void func_8006E2EC(Unk8006E2EC *this) {
    threadSwitchTo(&this->thread, 0);
}

/* The entry of the thread of gameMainTaskInit's object. */
void func_8006E310(s32 unused, void *object) {
    func_8006E330(object);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006E330);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006E600);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006E660);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006E948);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EA64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EBB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006ECA4);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8006ECEC(Unk8006ECEC *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006ED20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EF5C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EF64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EF6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EF90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EFC0);

/* Returns the word index of the array at 0x240. */
s32 func_8006EFE8(Unk8006ED20 *this, s32 index) {
    return this->unk240[index];
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006EFFC);

/* Returns a random number from the object's own generator. */
u32 func_8006F004(Unk8006ED20 *this) {
    return mersenneTwisterGenerate(&this->random);
}

/* Adds node at the end of the list at 0x228. */
void func_8006F024(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk228, node);
}

/* Adds node at the end of the list at 0x230. */
void func_8006F044(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk230, node);
}

/* Adds node at the end of the list at 0x238. */
void func_8006F064(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk238, node);
}

/* Adds entity, if there is one, to the group at 0x3C; returns it. */
Entity *func_8006F084(Unk8006ED20 *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->unk3C, entity);
    }
    return entity;
}

/* Adds entity, if there is one, to the group at 0x4C; returns it. */
Entity *func_8006F0B4(Unk8006ED20 *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->unk4C, entity);
    }
    return entity;
}

/* Adds an entity to the group at 0x5C; returns the group. */
EntityGroup *func_8006F0E4(Unk8006ED20 *this, Entity *entity) {
    return entityGroupAppend(&this->unk5C, entity);
}

/* Whether the group at 0x5C holds any entity. */
s32 func_8006F104(Unk8006ED20 *this) {
    return entityGroupGetCount(&this->unk5C) != 0;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F124);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F144);

/* Returns bit 4 of the flags at 0x2C8. */
s32 func_8006F190(Unk8006ED20 *this) {
    return (this->unk2C8 >> 4) & 1;
}

/* Returns bit 8 of the flags at 0x2C8. */
s32 func_8006F1A4(Unk8006ED20 *this) {
    return (this->unk2C8 >> 8) & 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F1B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F1C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F254);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F414);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F57C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F690);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006F780);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006FAF4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006FEB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8006FFD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007005C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800700C0);

/* An object Unk8006ED20 keeps at 0x2C by side (typed void * in game.h): its character at 0x8. */
typedef struct Unk800700C8 {
    /* 0x0 */ u8 unk0[8];
    /* 0x8 */ Unk80076F34 *chara;
} Unk800700C8;

/* Returns the character of side. */
Unk80076F34 *func_800700C8(Unk8006ED20 *this, s32 side) {
    return ((Unk800700C8 *)this->unk2C[side])->chara;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800700E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070140);

/* Creates the object of a virtual table slot that the stages override with their own (0x6C bytes here). */
void *func_800702E0(void *this, s32 unk, s32 arg) {
    return func_8007C17C(operatorNew(0x6C), arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070314);

/* The entry of the thread of func_8006ED20's object. */
void func_8007039C(s32 unused, Unk8006ED20 *object) {
    func_80071110(object);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800703BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070474);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800706A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070824);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070838);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800708C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800708F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070C28);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070C74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070DBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80070F40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071014);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800710B8);

/* Sets bit 3 of the flags at 0x2C8. */
void func_800710FC(Unk8006ED20 *this) {
    this->unk2C8 |= 8;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071110);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071460);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800715EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071738);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071894);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071A8C);

void func_80079268(Unk80076F34 *chara);
/* the constructor of the 0x59C-byte entity class Unk800CB7B0 (declared below) */
Entity *func_800CB7B0(void *this, s32 arg1, s32 arg2);
s32 func_80071D38(Unk8006ED20 *this, Entity *entity, s32 port);

/* Plays sounds 0x8009 and 0x42, then adds an entity of func_800CB7B0's class (10) and waits for it
 * (func_80071D38). */
void func_80071B74(Unk8006ED20 *this) {
    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    func_80071D38(this, func_8006F0B4(this, func_800CB7B0(entityOperatorNew(0x59C), 10, 0)), -1);
}

/* func_80071B74 with class 9, after func_80079268 on both sides' characters. */
void func_80071BF8(Unk8006ED20 *this) {
    Unk80076F34 *chara0;
    Unk80076F34 *chara1;

    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    /* The match depends on reading both characters before the calls. */
    chara0 = ((Unk800700C8 *)this->unk2C[0])->chara;
    chara1 = ((Unk800700C8 *)this->unk2C[1])->chara;
    func_80079268(chara0);
    func_80079268(chara1);
    func_80071D38(this, func_8006F0B4(this, func_800CB7B0(entityOperatorNew(0x59C), 9, 0)), -1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071C9C);

s32 func_80071D7C(Unk8006ED20 *this, u32 handle, s32 port);

/* func_80071D7C for the entity's handle: waits until it is gone or the port presses 0x800. */
s32 func_80071D38(Unk8006ED20 *this, Entity *entity, s32 port) {
    return func_80071D7C(this, entityGetHandle(entity), port);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071D7C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071E10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071E60);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071EC0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071EF8);

void func_80071F6C(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80071F74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072088);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072680);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072748);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007286C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007293C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072ADC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072B74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072C6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072CA0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072E20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072F2C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80072F80);

/* Picks a node of D_8006437C for the two vectors (func_80076474) with a temporary Unk80076444. */
void func_80072FE0(Vec3 *a, Vec3 *b) {
    Unk80076444 picker;

    func_80076444(&picker);
    func_80076474(&picker, a, b);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073024);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073130);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073218);

/* Returns the func_80075000 object of the character index. */
struct Unk80075000 *func_80073260(Unk8006ED20 *this, s32 index) {
    return this->unk24[index];
}

/* Returns the object at 0x2C of side; nothing calls it. */
void *func_80073274(Unk8006ED20 *this, s32 side) {
    return this->unk2C[side];
}

/* Sets bit 5 of the flags at 0x2C8. */
void func_80073288(Unk8006ED20 *this) {
    this->unk2C8 |= 0x20;
}

/* Clears bit 5 of the flags at 0x2C8. */
void func_8007329C(Unk8006ED20 *this) {
    this->unk2C8 &= ~0x20;
}

/* Sets bit 6 of the flags at 0x2C8. */
void func_800732B0(Unk8006ED20 *this) {
    this->unk2C8 |= 0x40;
}

/* Clears bit 6 of the flags at 0x2C8. */
void func_800732C4(Unk8006ED20 *this) {
    this->unk2C8 &= ~0x40;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800732D8);

void func_8007338C(void) {
}

void func_80073394(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007339C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800733A8);

/* Returns the object func_8006ED20 constructed. */
Unk8006ED20 *func_800733B4(void) {
    return D_800D377C;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800733C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800734AC);

/* Sets 0 to unk0 and 0xC to unkC, clearing 4 and 8. */
void func_800735E8(Unk800735E8 *this, s32 unk0, s32 unkC) {
    this->unk0 = unk0;
    this->unk4 = 0;
    this->unk8 = 0;
    this->unkC = unkC;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800735FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073684);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073694);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800736D8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073708);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073A44);

/* The counter at 0x64 of the HUD entity (func_80073684 clears it, func_80073708 draws it). */
typedef struct Unk80073684 {
    /* 0x0 */ s32 value;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} Unk80073684;

/* The HUD entity of a side that func_800733C4 constructs (virtual table D_800650CC). */
typedef struct Unk800733C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800735E8 unk18;
    /* 0x28 */ Unk800735E8 unk28;
    /* 0x38 */ void *unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 side; /* 0: func_80073AC0 draws it, else func_80073BB4 (func_80073A44) */
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ Unk80073684 counter;
    /* 0x70 */ s32 visible; /* func_80073A44 draws nothing while 0 */
    /* 0x74 */ s32 unk74;
} Unk800733C4;

/* the parts of the HUD, each drawn at (x, y) */
void func_80073C80(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073ED4(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073FB8(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_800740F0(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80074250(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80074528(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_800749BC(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073708(Unk80073684 *counter, u_long *ot, s32 x, s32 y);

/* Draws the HUD of the left side: its parts, then the counter at x 0xC8 (0xBC from 10 up), y 0x30. */
void func_80073AC0(Unk800733C4 *this, u_long *ot) {
    Unk80073684 *counter;
    s32 x;

    func_80073FB8(this, ot, 0x94, 0xCD);
    func_80074250(this, ot, 0xF, 0xD8);
    func_80074528(this, ot, 0x41, 0xCC);
    func_80073ED4(this, ot, 0x76, 0xBE);
    func_800740F0(this, ot, 0x43, 0xBE);
    func_800749BC(this, ot, 4, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    /* The match depends on the counter pointer: its address is computed before the test, as in the game. */
    counter = &this->counter;
    if (counter->value < 10) {
        x = 0xC8;
    } else {
        x = 0xBC;
    }
    func_80073708(counter, ot + 2, x, 0x30);
    func_80073C80(this, ot, 0xF, 0xBD);
}

/* Draws the HUD of the right side: func_80073AC0's parts but the first, mirrored, and the counter at x 0x1C. */
void func_80073BB4(Unk800733C4 *this, u_long *ot) {
    func_80074250(this, ot, 0xB9, 0xD8);
    func_80074528(this, ot, 0xB9, 0xCC);
    func_80073ED4(this, ot, 0xBF, 0xBE);
    func_800740F0(this, ot, 0xCD, 0xBE);
    func_800749BC(this, ot, 0xA0, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    func_80073708(&this->counter, ot + 2, 0x1C, 0x30);
    func_80073C80(this, ot, 0x102, 0xBD);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073C80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073D78);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073ED4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80073FB8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800740F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80074250);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80074528);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800747F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800749BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80074A88);

/* The destructor of the entity class of vtable D_800650CC, which func_800733C4 installs. */
void func_80074FAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80074FF4);

/* Clears an object and starts it (func_80075034 with 0); returns it. */
void *func_80075000(void *this) {
    func_80075090(this);
    func_80075034(this, 0);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075034);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007504C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075090);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800750C4);

/* The buttons pressed (0x202), through func_80075574. */
u32 func_80075100(Unk80075000 *this) {
    return func_80075574(this, this->pressed);
}

/* The buttons released (0x204), through func_80075574. */
u32 func_80075124(Unk80075000 *this) {
    return func_80075574(this, this->released);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075148);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007516C);

u32 func_8007559C(u32 buttons);
s32 func_800755B4(u32 held, u32 buttons, s32 any);

/* Looks back through the history, from the entry at 0x20C, for frames that hold buttons (all of them, or any
 * when any is set); stops at the first frame after them that does not, and then keeps that place at
 * 0x20C/0x210. Returns whether it found them. */
s32 func_800751A8(Unk80075000 *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->unk20C;
    s32 left = this->unk210;
    u32 wanted = func_80075574(this, buttons);
    u32 mask = func_8007559C(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on found = 1 coming last in the loop: GCC 2.95's loop rotation then puts it before
         * the test, as in the game. */
        if (!func_800755B4(held, wanted, any)) {
            if (found) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->unk20C = (i + 1) & 0xFF;
        this->unk210 = left + 1;
    }
    return found;
}

/* func_800751A8 that gives up at the first frame, before the buttons, that holds any of the other buttons of
 * the group (an empty frame does not stop it). */
s32 func_8007527C(Unk80075000 *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->unk20C;
    s32 left = this->unk210;
    u32 wanted = func_80075574(this, buttons);
    u32 mask = func_8007559C(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on masking held again here, and on found = 1 coming last (func_800751A8). */
        held &= mask;
        if (!func_800755B4(held, wanted, any)) {
            if (found || held != 0) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->unk20C = (i + 1) & 0xFF;
        this->unk210 = left + 1;
    }
    return found;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007536C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075454);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075574);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007559C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800755B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800755D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075674);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800756E4);

/* One of func_8007AAE4's 0x24-byte objects: func_80075784 fits it to points, and 0x20 keeps a pointer. */
typedef struct Unk80075764 {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ void *unk20;
} Unk80075764;

void func_80075784(Unk80075764 *this, VECTOR *points);

/* Keeps arg2 at 0x20, then func_80075784 with points. */
void func_80075764(Unk80075764 *this, VECTOR *points, void *arg2) {
    this->unk20 = arg2;
    func_80075784(this, points);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075784);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075848);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075968);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075AE0);

void func_80075B48(Unk80075EE4 *this, Unk80075EE4 *other);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075B48);

/* Adds node at the end of the list head; returns head. */
ListNode *func_80075C64(ListNode *head, ListNode *node) {
    listInsertAfter(head->prev, node);
    return head;
}

void func_80075CEC(ListNode *list, Unk80075EE4 *other);

/* func_80075CEC of list with every Unk80075EE4 of others. */
void func_80075C94(ListNode *list, ListNode *others) {
    ListNode *node = others->next;

    /* The match depends on the one iterator, stepped before the call (func_80075CEC keeps a next). */
    while (node != others) {
        Unk80075EE4 *part = (Unk80075EE4 *)node;

        node = node->next;
        func_80075CEC(list, part);
    }
}

/* func_80075B48 of every Unk80075EE4 of list with other. */
void func_80075CEC(ListNode *list, Unk80075EE4 *other) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        next = node->next;
        func_80075B48((Unk80075EE4 *)node, other);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075D44);

/* Calls the virtual method at 0x18 of every Unk80075EE4 of list with arg. */
void func_80075E7C(ListNode *list, s32 arg) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        Unk80075EE4 *part = (Unk80075EE4 *)node;

        next = node->next;
        part->vtable[3].func((u8 *)part + part->vtable[3].delta, arg);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075EE4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075F58);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80075FE4);

extern VtableEntry D_8006513C[];

/* Constructs the class of virtual table D_80065184 (Unk800761AC) at this: unlinked, with owner and unk10, its
 * words 0x14-0x2C cleared and flag bits 0 and 1 clear. Written on Unk80075EE4, which spells out the fields
 * game_classes.h's Unk800761AC leaves as gaps. */
Unk80075EE4 *func_800761AC(Unk80075EE4 *this, s32 unk10, void *owner) {
    this->vtable = D_80065184;
    this->node.next = &this->node;
    this->node.prev = &this->node;
    this->owner = owner;
    this->unk10 = unk10;
    this->unk14 = NULL;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20.vx = 0;
    this->unk20.vy = 0;
    this->unk20.vz = 0;
    this->unk2C = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076204);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076210);

/* Unk800761AC's destructor: takes it out of its list. */
void func_8007621C(Unk800761AC *this, s32 flags) {
    this->vtable = D_80065184;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076274);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007627C);

/* Sets bit 0 of the flags at 0xC to bit 0 of value; nothing calls it. */
void func_80076284(Unk800761AC *this, s32 value) {
    this->unkC = (this->unkC & ~1) | (value & 1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800762A0);

/* Returns bit 1 of the flags at 0xC; nothing calls it. */
s32 func_800762AC(Unk800761AC *this) {
    return (this->unkC >> 1) & 1;
}

/* Clears bit 1 of the flags at 0xC. */
void func_800762C0(Unk800761AC *this) {
    this->unkC &= ~2;
}

/* Adds node at the end of the list head (func_80075C64); nothing calls it. */
void func_800762D4(ListNode *head, ListNode *node) {
    func_80075C64(head, node);
}

/* Constructs an Unk80076308: only its virtual table, D_80065164. */
Unk80076308 *func_800762F4(Unk80076308 *this) {
    this->vtable = D_80065164;
    return this;
}

/* Unk80076308's destructor. */
void func_80076308(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The destructor of func_80075EE4's class (virtual table D_8006513C): Unk800761AC's, inlined. */
void func_8007633C(Unk800761AC *this, s32 flags) {
    this->vtable = D_80065184;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an Unk80075EE4 (virtual table D_8006513C) of owner: func_800761AC's fields, inlined, with
 * unk10 = 0, then flag bit 0 set. */
Unk80075EE4 *func_80076394(Unk80075EE4 *this, void *owner) {
    this->node.next = &this->node;
    this->node.prev = &this->node;
    this->owner = owner;
    this->unk10 = 0;
    this->unk14 = NULL;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20.vx = 0;
    this->unk20.vy = 0;
    this->unk20.vz = 0;
    this->unk2C = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    this->vtable = D_8006513C;
    this->flags &= ~1;
    this->flags |= 1;
    return this;
}

/* Copies the vector at 0x34 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800763F4(Unk800763F4 *this, Vec3 *out) {
    s32 x = this->unk34.vx;
    s32 y = this->unk34.vy;
    s32 z = this->unk34.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* Sets the vector at 0x34 from in (func_800763F4's counterpart).
 * The match depends on reading the three words before storing any, as vecCopy does. */
void func_80076414(Unk800763F4 *this, Vec3 *in) {
    s32 x = in->vx;
    s32 y = in->vy;
    s32 z = in->vz;

    this->unk34.vx = x;
    this->unk34.vy = y;
    this->unk34.vz = z;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076430);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076438);

/* Constructs the member at 0x260 of Unk80076F34: 0x78 is the first node of D_8006437C. */
Unk80076444 *func_80076444(Unk80076444 *this) {
    this->unk78 = (Unk80076A0C *)func_80076A00()->next;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076474);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076584);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076590);

/* Returns the matrix of the node of D_8006437C picked last. */
MATRIX *func_80076598(Unk80076444 *this) {
    return this->unk78->unkC;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800765AC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800765B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800765F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800766B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076888);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800768B8);

/* Whether point (x, y) is inside the box (func_8007691C). */
s32 func_800768F0(Unk8007691C *this, s32 *point) {
    return func_8007691C(this, point[0], point[1]);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007691C);

/* Swaps the four words at a with the four at b by xor (swapFourWords' body, a == b left alone) and
 * returns b. */
u32 *func_80076958(u32 *a, u32 *b) {
    u32 a0, b0, a1, b1;

    /* The match depends on b == a (the game compares b first). */
    if (b == a) {
        return b;
    }
    a0 = a[0];
    b0 = b[0];
    a1 = a[1];
    b1 = b[1];
    b0 ^= a0;
    b1 ^= a1;
    a0 ^= b0;
    a1 ^= b1;
    b0 ^= a0;
    b1 ^= a1;
    a[0] = a0;
    b[0] = b0;
    a[1] = a1;
    b[1] = b1;

    a0 = a[2];
    b0 = b[2];
    a1 = a[3];
    b1 = b[3];
    b0 ^= a0;
    b1 ^= a1;
    a0 ^= b0;
    a1 ^= b1;
    b0 ^= a0;
    b1 ^= a1;
    a[2] = a0;
    b[2] = b0;
    a[3] = a1;
    b[3] = b1;
    return b;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800769DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076A00);

/* Moves node to the front of the list D_8006437C. */
void func_80076A0C(Unk80076A0C *node) {
    listRemove(&node->link);
    listInsertAfter(&D_8006437C, &node->link);
}

/* Moves node to the back of the list D_8006437C. */
void func_80076A5C(Unk80076A0C *node) {
    listRemove(&node->link);
    node->link.next = &D_8006437C;
    node->link.prev = D_8006437C.prev;
    D_8006437C.prev->next = &node->link;
    D_8006437C.prev = &node->link;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076AAC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076AF4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076BB4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076C74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076CB4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80076F34);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800770AC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007720C);

/* func_8006EF90 for the character's index at 0x21C, adding arg. */
void func_800773F4(Unk80076F34 *this, s32 arg) {
    func_8006EF90(this->unk228, this->unk21C, arg);
}

/* func_8007F0F4 on the object at 0x238, with arg. */
void func_80077424(Unk80076F34 *this, s32 arg) {
    func_8007F0F4(this->unk238, arg);
}

/* func_8007F1FC on the object at 0x238, with arg. */
void func_80077448(Unk80076F34 *this, s32 arg) {
    func_8007F1FC(this->unk238, arg);
}

/* Calls func_80079350 when func_8007F5A8 on the object at 0x238 says so. */
void func_8007746C(Unk80076F34 *this) {
    if (func_8007F5A8(this->unk238)) {
        func_80079350(this);
    }
}

/* Clears the timed thing at 0x464. */
void func_800774A8(Unk80076F34 *this) {
    this->unk464.unk0 = 0;
    this->unk464.unk4 = 0;
    this->unk464.unk8 = 0;
    this->unk464.unkC = 0;
    this->unk464.unk10 = 0;
}

/* Starts the timed thing at 0x464. */
void func_800774C0(Unk80076F34 *this, s32 unk0, s32 unk4, s32 unk8) {
    this->unk464.unk0 = unk0;
    this->unk464.unk4 = unk4;
    this->unk464.unk8 = unk8;
    this->unk464.unkC = 0;
    this->unk464.unk10 = 0;
}

/* Runs the timed thing at 0x464 while it is set. */
void func_800774D8(Unk80076F34 *this) {
    if (this->unk464.unk0 != 0) {
        func_800C9790(this);
    }
}

/* Clears the timed thing at 0x478. */
void func_80077508(Unk80076F34 *this) {
    this->unk478.unk0 = 0;
    this->unk478.unk4 = 0;
    this->unk478.unk8 = 0;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Starts the timed thing at 0x478. */
void func_80077520(Unk80076F34 *this, s32 unk0, s32 unk4, s32 unk8) {
    this->unk478.unk0 = unk0;
    this->unk478.unk4 = unk4;
    this->unk478.unk8 = unk8;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Runs the timed thing at 0x478 while it is set. */
void func_80077538(Unk80076F34 *this) {
    if (this->unk478.unk0 != 0) {
        func_800C99B0(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077568);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077574);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077580);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007758C);

/* Returns bit 0 of the flags at 0x37C. */
s32 func_80077598(Unk80076F34 *this) {
    return this->flags & 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800775A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800775B0);

/* The character's position in whole units (0x384 keeps it in 1/256ths); returns out. */
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out) {
    out->vx = chara->position.vx >> 8;
    out->vy = chara->position.vy >> 8;
    out->vz = chara->position.vz >> 8;
    return out;
}

/* The vector at 0x394 in whole units (it is kept in 1/256ths); returns out. */
Vec3 *func_8007760C(Unk80076F34 *chara, Vec3 *out) {
    out->vx = chara->unk394.vx >> 8;
    out->vy = chara->unk394.vy >> 8;
    out->vz = chara->unk394.vz >> 8;
    return out;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077640);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007768C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077698);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800776A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800776C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800776D0);

/* Whether bit 0 of the first word of the character's table at 0x220 (func_80077698) is clear. */
s32 func_80077794(Unk80076F34 *this) {
    /* the game reads a halfword (lhu): game_classes.h types the first word as s32 */
    return !(*(u16 *)&func_80077698(this)->unk0 & 1);
}

/* Returns bit 1 of the flags at 0x37C. */
s32 func_800777BC(Unk80076F34 *this) {
    return (this->flags >> 1) & 1;
}

/* Adds the character's model to group. */
void func_800777D0(Unk80076F34 *this, Group *group) {
    /* The match depends on reading the model into a local first (the game loads it before moving group). */
    Model *model = this->model;

    groupAddChild(group, &model->group.object);
}

/* Takes the character's model out of the group it is a child of. */
void func_800777F8(Unk80076F34 *this) {
    listRemove(&this->model->group.object.link);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007782C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077848);

/* Returns bit 2 of the flags at 0x37C. */
s32 func_800778BC(Unk80076F34 *this) {
    return (this->flags >> 2) & 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800778D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800778FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077B20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077C70);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077CD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077E2C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80077F80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007814C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007819C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078254);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007835C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078418);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078478);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800784F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078540);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078578);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078660);

/* Sets 0x3EC to -0x401 (facing -1), and 0x3DC too when arg is set. */
void func_80078804(Unk80076F34 *this, s32 arg) {
    this->unk3EC = -0x401;
    if (arg) {
        this->unk3DC = -0x401;
    }
}

/* Sets 0x3EC to 0x401 (facing 1), and 0x3DC too when arg is set. */
void func_8007881C(Unk80076F34 *this, s32 arg) {
    this->unk3EC = 0x401;
    if (arg) {
        this->unk3DC = 0x401;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078834);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078840);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078848);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078878);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800788B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078910);

/* Keeps arg at 0x234 and passes it to the virtual method at 0x30 (func_800778FC
 * calls it with what func_80073260 returns). */
void func_80078C28(Unk80076F34 *this, struct Unk80075000 *arg) {
    Unk80076F34Vtable *vtable;

    this->unk234 = arg;
    vtable = UNK80076F34_VTABLE(this);
    vtable->unk30.func((u8 *)this + vtable->unk30.delta, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078C5C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078CE0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078D64);
s32 func_80078D64(Unk80076F34 *this);

/* Whether the first word of the object at 0x22C is 9 or 10. */
s32 func_80078DE0(Unk80076F34 *this) {
    s32 value = func_800792A0(this)->unk0;

    return value >= 9 && value <= 10;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078E08);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078E6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078EB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80078EF8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007925C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079268);

/* func_800807F4 on the object at 0x22C, with arg. */
void *func_80079274(Unk80076F34 *this, s32 arg) {
    return func_800807F4(this->unk22C, arg);
}

void func_80079298(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800792A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800792AC);

/* The first word of the object at 0x22C (func_800792A0). */
s32 func_800792B8(Unk80076F34 *this) {
    return func_800792A0(this)->unk0;
}

/* The word 0x230 points to (func_800792AC). */
s32 func_800792DC(Unk80076F34 *this) {
    return *func_800792AC(this);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079300);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079320);

/* While the object at 0x238 holds some (0x20 above 0): func_800774A8, func_80077508, then a new handle
 * for the character (func_8007F6FC; Unk8007F6FC is that object's class seen from its 0x30). */
void func_80079350(Unk80076F34 *this) {
    if (this->unk238->unk20 > 0) {
        func_800774A8(this);
        func_80077508(this);
        func_8007F6FC((Unk8007F6FC *)this->unk238);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800793A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079420);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800794CC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079534);

/* Unk80076F34's Chara (0x3F8) and the blend frames of its clip at 0x250 (0x24C) sit in gaps of
 * game_classes.h, which this file cannot change: these name them until the header does (RESULT.md). */
#define CHARA_OF(chara) (*(Chara **)((u8 *)(chara) + 0x3F8))
#define CLIP_BLEND_FRAMES_OF(chara) (*(s32 *)((u8 *)(chara) + 0x24C))

s32 func_80079A0C(Unk80076F34 *this);

/* Starts clip with the play count of the character's .loop file, keeping clip and blendFrames at 0x250/0x24C,
 * when func_80079A0C allows it; a negative clip stops the model. */
void func_800796F0(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->unk250 = clip;
    CLIP_BLEND_FRAMES_OF(this) = blendFrames;
    if (func_80079A0C(this)) {
        modelStartClip(model, clip, blendFrames, charaGetClipPlays(CHARA_OF(this), clip), frame);
        modelSetLoopFrame(model, charaGetLoopFrame(CHARA_OF(this), clip));
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800797B8);

/* Starts clip on the model (a negative one stops it), keeping it at 0x254; plays < 0 takes the count from the
 * character's .loop file. The match depends on the ternary: assigning plays loads it at the entry. */
void func_80079830(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame, s32 plays) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->unk254 = clip;
    modelStartClip(model, clip, blendFrames, plays < 0 ? charaGetClipPlays(CHARA_OF(this), clip) : plays, frame);
    modelSetLoopFrame(model, charaGetLoopFrame(CHARA_OF(this), clip));
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800798F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079950);

/* Whether the model is done with the clip at 0x250: stopped, no clip there, or playing another. */
s32 func_800799A0(Unk80076F34 *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->unk250 < 0) {
        return 1;
    }
    return modelGetClip(model) != this->unk250;
}

/* Whether the model is done with the clip at 0x254: stopped, no clip there, or playing another. */
s32 func_80079A0C(Unk80076F34 *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->unk254 < 0) {
        return 1;
    }
    return modelGetClip(model) != this->unk254;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079A78);

/* Sets bit 6 of the flags at 0x37C. */
void func_80079AE4(Unk80076F34 *this) {
    this->flags |= 0x40;
}

/* Sets bit 5 of the flags at 0x37C. */
void func_80079AF8(Unk80076F34 *this) {
    this->flags |= 0x20;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079B0C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079B14);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079B1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079B24);

/* Sets the character's 0x444 to arg, unless it holds arg's complement. */
void func_80079B2C(Unk80076F34 *this, s32 arg) {
    /* game_classes.h keeps 0x444 as padding bytes */
    s32 *unk444 = (s32 *)this->unk444;

    if (~*unk444 != arg) {
        *unk444 = arg;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079B4C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079BD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079C1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079CF4);

/* Returns bit 3 of the flags at 0x37C, or 0 while func_80078D64 holds. */
s32 func_80079D94(Unk80076F34 *this) {
    if (func_80078D64(this) != 0) {
        return 0;
    }
    return (this->flags >> 3) & 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079DD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079E10);

/* func_8007FB9C on the object at 0x22C. */
s32 func_80079E2C(Unk80076F34 *this) {
    return func_8007FB9C(this->unk22C);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079E50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80079FEC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A044);

/* Adds the bits func_8007A044 returns for arg to 0x448. */
void func_8007A0B4(Unk80076F34 *this, s32 arg) {
    this->unk448 |= func_8007A044(this, arg);
}

/* Stores what func_8007A044 returns for arg at 0x448. */
void func_8007A0E4(Unk80076F34 *this, s32 arg) {
    this->unk448 = func_8007A044(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A10C);

/* Counts up 0x340. */
void func_8007A118(Unk80076F34 *this) {
    this->unk340++;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A12C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A158);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A3EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A4A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A598);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A650);

/* Writes the position of this one's character, in whole units, to out and returns it. */
Vec3 *func_8007A69C(Unk80076F34 *this, Vec3 *out) {
    return func_800775D8(func_8007AEEC(this), out);
}

/* As func_8007A69C, through func_8007A650. */
Vec3 *func_8007A6CC(Unk80076F34 *this, Vec3 *out) {
    return func_8007A650(func_8007AEEC(this), out);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A6FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A7DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A874);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A920);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A9B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007A9FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007AAE4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ACC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ACD0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ACDC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ACE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ACF4);

/* Adds entity, if there is one, to group 0x4C of func_8006ED20's object at 0x228.
 * It returns nothing: func_800B94AC puts a constant in $v0 right after calling it,
 * which a call with a value would keep it from. */
void func_8007AD4C(Unk80076F34 *this, Entity *entity) {
    func_8006F0B4(this->unk228, entity);
}

void func_8007ADCC(Unk80076F34 *this, MeshPart *part, u32 handle);

/* func_8007ADCC with the part of chara's model at index and chara's handle. */
void func_8007AD70(Unk80076F34 *this, Unk80076F34 *chara, s32 index) {
    MeshPart *part = chara->model->parts[index];

    func_8007ADCC(this, part, entityGetHandle(&chara->entity));
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007ADCC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007AE34);

/* Whether the first word of the object at 0x22C is 11 or 12. */
s32 func_8007AEA8(Unk80076F34 *this) {
    s32 value = func_800792A0(this)->unk0;

    return value >= 11 && value <= 12;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007AED0);

Unk80076F34 *func_80073218(Unk8006ED20 *this, Unk80076F34 *chara);

/* The Unk80076F34 that func_80073218 gives for the object at 0x228. */
Unk80076F34 *func_8007AEEC(Unk80076F34 *this) {
    return func_80073218(this->unk228, this);
}

/* When arg is 0, starts the model's clip 0x12 (func_800796F0). */
void func_8007AF14(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0x12, 2, 0);
    }
}

/* A virtual function of Unk80076F34 (entry 0x48 of D_8006530C): with arg 0, func_800796F0,
 * func_80078804 and func_8007A6FC; then func_8007835C with minus the table's unk8. */
void func_8007AF40(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 1, 4, 0);
        func_80078804(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 2 : 3);
    }
    func_8007835C(this, -func_80077698(this)->unk8);
}

/* A virtual function of Unk80076F34 (entry 0x50 of D_8006530C): with arg 0, func_800796F0,
 * func_8007881C and func_8007A6FC; then func_8007835C with the table's unk8. */
void func_8007AFBC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 1, 4, 0);
        func_8007881C(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 2 : 3);
    }
    func_8007835C(this, func_80077698(this)->unk8);
}

/* A virtual function of Unk80076F34 (entry 0x58 of D_8006530C): with arg 0, func_800796F0, takes
 * the table's unkC (scaled by unk3C4, in 16.16) from unk3B4.vx, then func_80078804 and
 * func_8007A6FC; always sets flag 0x80. */
void func_8007B034(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        s32 delta;

        func_800796F0(this, 2, 1, 0);
        delta = ((s64)func_80077698(this)->unkC * this->unk3C4) >> 16;
        this->unk3B4.vx -= delta;
        func_80078804(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 3 : 4);
    }
    this->flags |= 0x80;
}

/* A virtual function of Unk80076F34 (entry 0x60 of D_8006530C): with arg 0, func_800796F0, adds
 * the table's unkC (scaled by unk3C4, in 16.16) to unk3B4.vx, then func_8007881C and
 * func_8007A6FC; always sets flag 0x80. */
void func_8007B0E0(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        s32 delta;

        func_800796F0(this, 2, 1, 0);
        /* The match depends on delta: as one expression, the sum takes unk3B4's register. */
        delta = ((s64)func_80077698(this)->unkC * this->unk3C4) >> 16;
        this->unk3B4.vx += delta;
        func_8007881C(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 3 : 4);
    }
    this->flags |= 0x80;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007B18C);

/* Starts the model's clip 0x13 (func_800796F0). */
void func_8007B298(Unk80076F34 *this) {
    func_800796F0(this, 0x13, 2, 0);
}

/* When arg is 0, starts the model's clip 0xC (func_800796F0). */
void func_8007B2C0(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0xC, 3, 0);
    }
}

/* When arg is 0, starts the model's clip 0x15 (func_800796F0). */
void func_8007B2EC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0x15, 3, 0);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007B318);

/* When arg is 0, starts the model's clip 0xD (func_800796F0). */
void func_8007B41C(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0xD, 1, 0);
    }
}

/* Starts the model's clip 0xE (func_800796F0). */
void func_8007B448(Unk80076F34 *this) {
    func_800796F0(this, 0xE, 1, 0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007B470);

/* A virtual function of Unk80076F34 (entry 0xA0 of D_8006530C): func_80079AE4, func_80079830
 * with 5 when arg is 0, then func_8007B470 with the table's entries[0]. */
void func_8007B6A0(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 5, 0, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[0], 1);
}

/* A virtual function of Unk80076F34 (entry 0xA8 of D_8006530C): func_80079AE4, func_80079830
 * with 6 and func_8007835C when arg is 0, then func_8007B470 with the table's entries[1]. */
void func_8007B70C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 6, 0, 0, -1);
        func_8007835C(this, 0);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[1], 1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007B784);

/* A virtual function of Unk80076F34 (entry 0xB8 of D_8006530C): func_80079AE4, func_80079830
 * with 31 and func_8007835C when arg is 0, then func_8007B470 with the table's entries[4]. */
void func_8007B80C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 31, 0, 0, -1);
        func_8007835C(this, 0);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[4], 1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007B884);

/* A virtual function of Unk80076F34 (entry 0xC8 of D_8006530C): func_80079AE4, func_80079830
 * with 20 when arg is 0, then func_8007B470 with the table's entries[5]. */
void func_8007B90C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 20, 0, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[5], 1);
}

/* func_80079AE4, then, when arg is 0, starts the model's clip 0xB (func_80079830). */
void func_8007B978(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 0xB, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x16 (func_80079830). */
void func_8007B9C8(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x16, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x18 (func_80079830). */
void func_8007B9FC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x18, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x1A (func_80079830). */
void func_8007BA30(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x1A, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x1B (func_80079830). */
void func_8007BA64(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x1B, 2, 0, -1);
    }
}

/* A virtual function of Unk80076F34 (entry 0xF8 of D_8006530C): func_80079830 with 9 when arg is 0,
 * then func_8007B470 with the table's entries[8]. */
void func_8007BA98(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 9, 2, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[8], 1);
}

/* A virtual function of Unk80076F34 (entry 0x100 of D_8006530C): func_80079830 with 10 and clearing the
 * y at 0x3A8 when arg is 0, then func_8007B470 with the table's entries[6]. */
void func_8007BAFC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 10, 2, 0, -1);
        this->unk3A4.vy = 0;
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[6], 1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BB64);

/* When arg is 0, starts the model's clip 0x17 (func_80079830). */
void func_8007BBE8(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x17, 2, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x19 (func_80079830). */
void func_8007BC1C(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x19, 2, 0, -1);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BC50);

/* Constructs the class of virtual table D_800652EC: the three words, then the vector at 0x10.
 * The match depends on reading the vector's three words before storing any, as vecCopy does. */
Unk8007BD8C *func_8007BD8C(Unk8007BD8C *this, struct Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC) {
    s32 x, y, z;

    this->vtable = D_800652EC;
    this->chara = chara;
    this->unk8 = unk8;
    this->unkC = unkC;
    x = unk10->vx;
    y = unk10->vy;
    z = unk10->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
    return this;
}

/* Sets the words at 0x8 and 0xC. */
void func_8007BDC8(Unk8007BD8C *this, s32 unk8, s32 unkC) {
    this->unk8 = unk8;
    this->unkC = unkC;
}

/* Sets the vector at 0x10, reading all three components before writing any. */
void func_8007BDD4(Unk8007BD8C *this, VECTOR *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
}

/* Unk8007BD8C's first method: adds its count (saturating at 0x3FFFF), its 0xC and its vector into sum,
 * whose flags it replaces with its own. */
void func_8007BDF0(Unk8007BD8C *this, Unk80075EE4 *sum) {
    u32 flags = this->unk8 & ~0x3FFFF;
    /* The match depends on starting count with this object's part, then adding
     * sum's: count then lives in $a3 */
    s32 count = this->unk8 & 0x3FFFF;
    s32 x, y, z;

    count += sum->unk18 & 0x3FFFF;
    if (count > 0x3FFFF) {
        count = 0x3FFFF;
    }
    sum->unk18 = flags | count;
    sum->unk1C += this->unkC;
    /* The match depends on vecAddInPlace's shape: the three words read before any is written */
    x = sum->unk20.vx;
    y = sum->unk20.vy;
    z = sum->unk20.vz;
    x += this->unk10.vx;
    y += this->unk10.vy;
    z += this->unk10.vz;
    sum->unk20.vx = x;
    sum->unk20.vy = y;
    sum->unk20.vz = z;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BE78);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BF00);

/* Returns bit 1 of the flags at 0x20; nothing calls it. */
s32 func_8007BF0C(Unk8007BF20 *this) {
    return (this->flags >> 1) & 1;
}

/* Constructs an Unk8007BF20 on func_8007BD8C's (the same arguments) and clears bits 0 and 1 of its
 * flags. */
Unk8007BF20 *func_8007BF20(Unk8007BF20 *this, Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC) {
    func_8007BD8C(&this->base, chara, unk8, unk10, unkC);
    this->base.vtable = D_800652CC;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BF74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BF84);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007BF90);

/* Two counters (func_8007BF74 clears them, func_8007BF84 sets them, func_8007BF90 scales them). */
typedef struct Unk8007BF74 {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
} Unk8007BF74;

void func_8007C028(Unk8007BF74 *this, Unk80076F34 *chara);

/* Passes the first counter to chara (func_800773F4), the second with func_8007C028, and clears the first. */
void func_8007BFE0(Unk8007BF74 *this, Unk80076F34 *chara) {
    func_800773F4(chara, this->unk0);
    func_8007C028(this, chara);
    this->unk0 = 0;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007C028);

/* Constructs an Unk8007C084 with its word (virtual table D_800652AC). */
Unk8007C084 *func_8007C084(Unk8007C084 *object, s32 unk0) {
    object->vtable = D_800652AC;
    object->unk0 = unk0;
    return object;
}

/* Unk8007C084's destructor. */
void func_8007C09C(Unk8007C084 *this, s32 flags) {
    this->vtable = D_800652AC;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007C0D0);

void func_8007C104(void) {
}

void func_8007C10C(void) {
}

/* The destructor of the class of virtual table D_800652EC: Unk80076308's, inlined. */
void func_8007C114(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The destructor of the class of virtual table D_800652CC: Unk80076308's, inlined. */
void func_8007C148(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007C17C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007C1EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007EDD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007EE80);

/* The destructor of func_8007C17C's object (virtual table D_8006569C): Unk8007C084's. */
void func_8007EE98(Unk8007C084 *this, s32 flags) {
    func_8007C09C(this, flags);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007EEB8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007EF4C);

/* The object of func_8007EEB8 (virtual table D_800656BC, at 0x38 after its first fields as g++ places it): two
 * characters it owns, the first again at 8. */
typedef struct Unk8007EEB8 {
    /* 0x00 */ Unk80076F34 *chara;
    /* 0x04 */ Unk80076F34 *other; /* may be chara itself */
    /* 0x08 */ Unk80076F34 *unk8;
    /* 0x0C */ u8 unkC[0x38 - 0xC];
    /* 0x38 */ VtableEntry *vtable;
} Unk8007EEB8;

extern VtableEntry D_800656BC[];

/* The destructor of func_8007EEB8's object: deletes its character, and the other one when it is not the same. */
void func_8007EFC0(Unk8007EEB8 *this, s32 flags) {
    this->vtable = D_800656BC;
    entityDelete(&this->chara->entity);
    /* The match depends on this operand order (0 loads first). */
    if (this->chara != this->other) {
        entityDelete(&this->other->entity);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F068);

/* Adds amount to 0x20, kept between 0 and 0x1C. */
void func_8007F0F4(Unk8007F184 *this, s32 amount) {
    amount += this->unk20;
    if (amount > this->unk1C) {
        amount = this->unk1C;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->unk20 = amount;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F124);

/* Copies field 0x14 of what func_80077698 returns for the character to 0x28. */
void func_8007F184(Unk8007F184 *this) {
    this->unk28 = func_80077698(this->base.unk8)->unk14;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F1B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F1C0);

/* Adds amount to 0x24, kept between 0 and 0x10000. */
void func_8007F1FC(Unk8007F184 *this, s32 amount) {
    amount += this->unk24;
    if (amount > 0x10000) {
        amount = 0x10000;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->unk24 = amount;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F22C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F300);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F59C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F5A8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F674);

/* Gives the character a new handle (entityRenewHandle) and sets 0x30. */
void func_8007F6FC(Unk8007F6FC *this) {
    entityRenewHandle(&this->base.unk8->entity);
    this->unk30 = 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F730);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F7A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F7F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F89C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F8E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F8F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F8FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F908);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F914);

/* Constructs an Unk8007F920 with its value (its virtual table D_80068154, 4 cleared). */
Unk8007F920 *func_8007F920(Unk8007F920 *this, s32 unk0) {
    this->vtable = D_80068154;
    this->unk0 = unk0;
    this->unk4 = 0;
    return this;
}

/* Returns the object at 0x22C of the character at 8. */
Unk8007F920 *func_8007F93C(Unk8007F920 *this) {
    return this->unk8->unk22C;
}

/* Returns the pointer at 0x230 of the character at 8 (func_800792AC's; the header types it s32). */
s32 *func_8007F950(Unk8007F920 *this) {
    return (s32 *)this->unk8->unk230;
}

/* Whether the word the character's 0x230 points to is 1. */
s32 func_8007F964(Unk8007F920 *this) {
    return *func_8007F950(this) == 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F98C);

/* Whether the character's last input (func_800750C4) holds button 0x4. */
s32 func_8007F9AC(Unk8007F920 *this) {
    return (func_800750C4(func_800776C4(this->unk8)) & 0x4) != 0;
}

void func_8007F9DC(Unk8007F920 *this);
INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007F9DC);

/* Whether the character's last input (func_800750C4) holds PAD_DOWN. */
s32 func_8007FAC0(Unk8007F920 *this) {
    return (func_800750C4(func_800776C4(this->unk8)) & PAD_DOWN) != 0;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007FAF0);

/* Returns 0x250 of the character at 8. */
s32 func_8007FB9C(Unk8007F920 *this) {
    return this->unk8->unk250;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007FBB0);

/* Calls func_80078804 for the Unk80076F34 of an Unk8007F920 when func_8007F964 holds. */
void func_8007FBBC(Unk8007F920 *this) {
    if (func_8007F964(this)) {
        func_80078804(this->unk8, 0);
    }
}

/* Calls func_8007881C for the Unk80076F34 of an Unk8007F920 when func_8007F964 holds. */
void func_8007FBF4(Unk8007F920 *this) {
    if (func_8007F964(this)) {
        func_8007881C(this->unk8, 0);
    }
}

/* func_800796F0 on the character at 8, from frame 0. */
void func_8007FC2C(Unk8007F920 *this, s32 clip, s32 blendFrames) {
    func_800796F0(this->unk8, clip, blendFrames, 0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007FC50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8007FC7C);

/* Calls the character's virtual method at 0x38 with 0x4, then increments 0x4. */
void func_8007FCB8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk38.func((u8 *)chara + vtable->unk38.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x40 with 0x4, then increments 0x4. */
void func_8007FCFC(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk40.func((u8 *)chara + vtable->unk40.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x48 with 0x4, then increments 0x4. */
void func_8007FD40(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk48.func((u8 *)chara + vtable->unk48.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x50 with 0x4, then increments 0x4. */
void func_8007FD84(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk50.func((u8 *)chara + vtable->unk50.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x58 with 0x4, then increments 0x4. */
void func_8007FDC8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk58.func((u8 *)chara + vtable->unk58.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x60 with 0x4, then increments 0x4. */
void func_8007FE0C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk60.func((u8 *)chara + vtable->unk60.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x68 with 0x4, then increments 0x4. */
void func_8007FE50(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk68.func((u8 *)chara + vtable->unk68.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x70 with 0x4, then increments 0x4. */
void func_8007FE94(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk70.func((u8 *)chara + vtable->unk70.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x78 with 0x4, then increments 0x4. */
void func_8007FED8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk78.func((u8 *)chara + vtable->unk78.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x80 with 0x4, then increments 0x4. */
void func_8007FF1C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk80.func((u8 *)chara + vtable->unk80.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x88 with 0x4, then increments 0x4. */
void func_8007FF60(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk88.func((u8 *)chara + vtable->unk88.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x90 with 0x4, then increments 0x4. */
void func_8007FFA4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk90.func((u8 *)chara + vtable->unk90.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x98 with 0x4, then increments 0x4. */
void func_8007FFE8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk98.func((u8 *)chara + vtable->unk98.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xA0 with 0x4, then increments 0x4. */
void func_8008002C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkA0.func((u8 *)chara + vtable->unkA0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xA8 with 0x4, then increments 0x4. */
void func_80080070(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkA8.func((u8 *)chara + vtable->unkA8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xB0 with 0x4, then increments 0x4. */
void func_800800B4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkB0.func((u8 *)chara + vtable->unkB0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xB8 with 0x4, then increments 0x4. */
void func_800800F8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkB8.func((u8 *)chara + vtable->unkB8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xC0 with 0x4, then increments 0x4. */
void func_8008013C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkC0.func((u8 *)chara + vtable->unkC0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xC8 with 0x4, then increments 0x4. */
void func_80080180(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkC8.func((u8 *)chara + vtable->unkC8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xD0 with 0x4, then increments 0x4. */
void func_800801C4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkD0.func((u8 *)chara + vtable->unkD0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xD8 with 0x4, then increments 0x4. */
void func_80080208(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkD8.func((u8 *)chara + vtable->unkD8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xE0 with 0x4, then increments 0x4. */
void func_8008024C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkE0.func((u8 *)chara + vtable->unkE0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xE8 with 0x4, then increments 0x4. */
void func_80080290(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkE8.func((u8 *)chara + vtable->unkE8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xF0 with 0x4, then increments 0x4. */
void func_800802D4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkF0.func((u8 *)chara + vtable->unkF0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xF8 with 0x4, then increments 0x4. */
void func_80080318(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkF8.func((u8 *)chara + vtable->unkF8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x100 with 0x4, then increments 0x4. */
void func_8008035C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk100.func((u8 *)chara + vtable->unk100.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x108 with 0x4, then increments 0x4. */
void func_800803A0(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk108.func((u8 *)chara + vtable->unk108.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x110 with 0x4, then increments 0x4. */
void func_800803E4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk110.func((u8 *)chara + vtable->unk110.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x118 with 0x4, then increments 0x4. */
void func_80080428(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk118.func((u8 *)chara + vtable->unk118.delta, this->unk4++);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008046C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080510);

/* Calls the character's virtual method at 0x20, then makes this a func_800819E0 object in place. */
void func_80080590(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk20.func((u8 *)chara + vtable->unk20.delta);
    func_800819E0(this);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800805DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080670);

/* Constructs the object of virtual table D_8006811C with 0x11 at 0 and starts the
 * character's clip 0x1D, blended over 10 frames. */
Unk8007F920 *func_80080730(Unk8007F920 *this) {
    func_8007F920(this, 0x11);
    this->vtable = D_8006811C;
    func_8007FC2C(this, 0x1D, 10);
    return this;
}

/* Constructs an object of the class of vtable D_8006811C, which derives from func_8007F920's. */
Unk8007F920 *func_80080778(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_8006811C;
    return this;
}

/* Constructs the object of virtual table D_8006811C (func_80080778) with 0x12 at 0
 * and starts the character's clip 0x1C, blended over 10 frames. */
Unk8007F920 *func_800807AC(Unk8007F920 *this) {
    func_80080778(this, 0x12);
    this->vtable = D_8006811C;
    func_8007FC2C(this, 0x1C, 10);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800807F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080874);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800808EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080968);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080ADC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080B4C);

/* Constructs an object of the class of vtable D_80068074 with func_800808EC and 2, then calls
 * func_8007FE94 and func_8007F9DC. */
Unk8007F920 *func_80080BA8(Unk8007F920 *this) {
    func_800808EC(this, 2);
    this->vtable = D_80068074;
    func_8007FE94(this);
    func_8007F9DC(this);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080BF0);

/* Constructs the class of virtual table D_80068004 on func_8007F920's. */
Unk80080C5C *func_80080C5C(Unk80080C5C *this, s32 arg1, s32 arg2) {
    func_8007F920(&this->base, arg1);
    this->base.vtable = D_80068004;
    this->unk10 = arg2;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080CA0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080D38);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080E68);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080EBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080F1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80080FDC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081054);

/* The method at 0x10 of D_80067F5C and D_80067F94: func_80080BF0(0) when func_8007F964 holds and arg is
 * positive. */
void func_800811B8(Unk8007F920 *this, s32 arg) {
    if (func_8007F964(this) && arg > 0) {
        func_80080BF0(this, 0);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081200);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081264);

/* Constructs an object of the class of vtable D_80068074 with func_800808EC and 9, then calls
 * func_8007FED8. */
Unk8007F920 *func_80081384(Unk8007F920 *this) {
    func_800808EC(this, 9);
    this->vtable = D_80068074;
    func_8007FED8(this);
    return this;
}

/* Constructs an object of the class of vtable D_80068074, which derives from func_800808EC's. */
Unk8007F920 *func_800813C4(Unk8007F920 *this, s32 arg) {
    func_800808EC(this, arg);
    this->vtable = D_80068074;
    return this;
}

/* Constructs an object of the class of vtable D_80068074 with func_800813C4 and 10, then calls
 * func_8007FF1C. */
Unk8007F920 *func_800813F8(Unk8007F920 *this) {
    func_800813C4(this, 10);
    this->vtable = D_80068074;
    func_8007FF1C(this);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081438);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081528);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800815C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081618);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800816B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081734);

/* Constructs an object of the class of vtable D_80067E7C with func_8007F920 and 15, then calls
 * func_8007FCFC. */
Unk8007F920 *func_800817B4(Unk8007F920 *this) {
    func_8007F920(this, 15);
    this->vtable = D_80067E7C;
    func_8007FCFC(this);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800817F4);

/* Constructs an object of the class of vtable D_80067E44 with func_8007F920 and 16, calls
 * func_8007FF60 and clears 0x10. */
Unk80081874 *func_80081874(Unk80081874 *this) {
    func_8007F920(&this->base, 16);
    this->base.vtable = D_80067E44;
    func_8007FF60(&this->base);
    this->unk10 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800818B8);

/* The method at 0x28 of D_80067E44: func_8007F1B8 on the character's object at 0x238, then
 * func_80081438. */
void func_80081998(Unk8007F920 *this, s32 arg) {
    func_8007F1B8(func_8007758C(this->unk8));
    func_80081438(this, arg);
}

/* Constructs an object of the class of vtable D_80067E0C with func_8007F920 and 1, and clears the
 * character's 0x448. */
Unk8007F920 *func_800819E0(Unk8007F920 *this) {
    func_8007F920(this, 1);
    this->vtable = D_80067E0C;
    this->unk8->unk448 = 0;
    return this;
}

/* Constructs an object of the class of vtable D_80067E0C, which derives from func_8007F920's. */
Unk8007F920 *func_80081A20(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_80067E0C;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081A54);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80081B84);

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x17, and if func_800778D0 does for 2, turns into state 0x1E stepping with
 * D_80067CCC; with any other value, unless func_800798F4 says so for 0x16,
 * into state 0x1D with D_80067CD4. Either way then func_800788B4 with 10, 100. */
void func_80081E18(Unk8007F920 *this) {
    s32 value = func_8007FB9C(this);
    Unk80076F34 *chara = this->unk8;

    switch (value) {
    case 3:
    case 4:
        if (!func_800798F4(chara, 0x17) && func_800778D0(chara, 2)) {
            func_8008209C(this, 0x1E, D_80067CCC);
            func_800788B4(chara, 0xA, 100);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x16)) {
            func_8008209C(this, 0x1D, D_80067CD4);
            func_800788B4(chara, 0xA, 100);
        }
        break;
    }
}

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x19, and if func_800778D0 does for 3, turns into state 0x20 stepping with
 * D_80067CDC; with any other value, unless func_800798F4 says so for 0x18,
 * into state 0x1F with D_80067CE4. Either way then func_800788B4 with 11, 1000. */
void func_80081EEC(Unk8007F920 *this) {
    s32 value = func_8007FB9C(this);
    Unk80076F34 *chara = this->unk8;

    switch (value) {
    case 3:
    case 4:
        /* the game reads 0x8 again here, where its siblings use chara */
        if (!func_800798F4(chara, 0x19) && func_800778D0(this->unk8, 3)) {
            func_8008209C(this, 0x20, D_80067CDC);
            func_800788B4(chara, 0xB, 1000);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x18)) {
            func_8008209C(this, 0x1F, D_80067CE4);
            func_800788B4(chara, 0xB, 1000);
        }
        break;
    }
}

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x1B, and if func_800778D0 does for 4, turns into state 0x22 stepping with
 * D_80067CEC; with any other value, unless func_800798F4 says so for 0x1A,
 * into state 0x21 with D_80067CF4. Either way then func_800788B4 with 12, 3000. */
void func_80081FC4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    s32 value = func_8007FB9C(this);

    switch (value) {
    case 3:
    case 4:
        if (!func_800798F4(chara, 0x1B) && func_800778D0(chara, 4)) {
            func_8008209C(this, 0x22, D_80067CEC);
            func_800788B4(chara, 0xC, 3000);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x1A)) {
            func_8008209C(this, 0x21, D_80067CF4);
            func_800788B4(chara, 0xC, 3000);
        }
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008209C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082158);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082270);

/* func_80081A54 unless func_8007F98C holds. */
void func_800822CC(Unk8007F920 *this, s32 arg) {
    if (!func_8007F98C(this)) {
        func_80081A54(this, arg);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008230C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800823EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800824B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800825AC);

/* Turns the object into the class of virtual table D_80067D54 (func_80082690 constructs it in place). */
Unk8007F920 *func_8008261C(Unk8007F920 *this) {
    return func_80082690(this);
}

/* Resets the object (func_800819E0), then calls func_8007FC50 on it with -1 and 0. */
void func_8008263C(void *this) {
    func_800819E0(this);
    func_8007FC50(this, -1, 0);
}

/* Turns the object into the class of virtual table D_80067D14 (func_800826E8 constructs it in place). */
Unk8007F920 *func_80082670(Unk8007F920 *this) {
    return func_800826E8(this);
}

/* Constructs the state 0x15 stepping with D_80067D04 (virtual table D_80067D54), then
 * func_800788B4 on the character with 1, 100. */
Unk8007F920 *func_80082690(Unk8007F920 *this) {
    func_800823EC(this, 0x15, D_80067D04);
    this->vtable = D_80067D54;
    func_800788B4(this->unk8, 1, 100);
    return this;
}

/* Constructs the state 0x16 stepping with D_80067D0C (virtual table D_80067D14), then
 * func_800788B4 on the character with 2, 200. */
Unk8007F920 *func_800826E8(Unk8007F920 *this) {
    func_800823EC(this, 0x16, D_80067D0C);
    this->vtable = D_80067D14;
    func_800788B4(this->unk8, 2, 200);
    return this;
}

void func_80082740(void) {
}

void func_80082748(void) {
}

void func_80082750(void) {
}

void func_80082758(void) {
}

void func_80082760(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082768);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082770);

/* Constructs an object of the class of vtable D_8006811C, which derives from func_8007F920's. */
Unk8007F920 *func_8008277C(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_8006811C;
    return this;
}

/* Constructs an object of the class of vtable D_800680E4, which derives from func_8007F920's. */
Unk8007F920 *func_800827B0(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_800680E4;
    return this;
}

void func_800827E4(void) {
}

/* Constructs an entity of the class of vtable D_8006843C, which derives from Unk80076F34 and
 * gives it the table D_800681DC. */
Unk80076F34 *func_800827EC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006843C;
    func_80079B24(this, D_800681DC);
    return this;
}

/* The override in virtual table D_8006843C: func_80079B2C with 0. */
void func_80082830(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

/* Creates an entity of func_80082EE8's class (0x34 bytes) with g++'s new, and hands it to func_8007AD4C: it
 * launches the Unk80076F34 of func_8007AEEC with vel. The match depends on this helper: its vel parameter keeps
 * &vel in a saved register across allocEntityMemory. */
static inline void newLaunch(Unk80076F34 *this, Vec3 *vel, s32 side) {
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        func_80082EE8(entityOperatorNewAt(0x34, child), this, vel, 0x808C2000, side);
    }
    func_8007AD4C(this, child);
}

/* A virtual function of Unk80076F34: func_8007B978; before step 5, func_8007835C and copies unk3A4 and unk3B4
 * to the Unk80076F34 of func_8007AEEC; at step 0, func_80079274 and func_800796F0 on it; at step 5, launches it
 * up and toward func_80079E10's side with newLaunch. */
void func_80082850(Unk80076F34 *this, s32 step) {
    Unk80076F34 *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    func_8007B978(this, step);
    other = func_8007AEEC(this);
    if (step < 5) {
        func_8007835C(this, 0);
        /* The match depends on each vector read into x, y and z before it is stored. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        other->unk3A4.vx = x;
        other->unk3A4.vy = y;
        other->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        other->unk3B4.vx = x;
        other->unk3B4.vy = y;
        other->unk3B4.vz = z;
    }
    switch (step) {
    case 0:
        func_80079274(other, 5);
        func_800796F0(other, 0, 0, 0);
        break;
    case 5:
        side = func_80079E10(this);
        vel.vx = side * 5460;
        vel.vy = -21840;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006843C: func_8007B9C8, sets bit 6 of
 * 0x37C, then for arg 0 plays sound 0x77, and for arg 13 plays sound 0x73 and calls func_80082AF8. */
void func_800829A0(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x77);
        func_80079B2C(this, 1);
        break;
    case 13:
        func_80078540(this, 0x73);
        func_80079B2C(this, 0);
        func_80082AF8(this);
        break;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006843C: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 0 plays sound 0x77, arg 7 plays sound 0x73 and calls func_80082AF8; below 8 the vector at
 * 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_80082A30(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x77);
        func_80079B2C(this, 1);
        break;
    case 7:
        func_80078540(this, 0x73);
        func_80079B2C(this, 0);
        func_80082AF8(this);
        break;
    }
    if (arg < 8) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082AF8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082C10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082DBC);

/* Creates an entity of func_80083700's class (0x4C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_80082E8C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x4C);

    if (child != NULL) {
        func_80083700(entityOperatorNewAt(0x4C, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082EE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80082FF0);

/* Constructs func_800C7024's entity with 0x80, 0x5A81C00, 0xA00, 3, then the virtual tables
 * D_800683EC (and D_800683CC for the base at 0x18); clears 0xC0. */
Unk80083108 *func_80083108(Unk80083108 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C7024(&this->base, chara, position, velocity, 0x80, 0x5A81C00, 0xA00, 3);
    this->base.parts.base.vtable = D_800683CC;
    this->base.parts.entity.vtable = &D_800683EC;
    this->unkC0 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80083170);

/* Spawns func_800833F8's entity (0x38 bytes) at the position, as a sibling, then runs the base
 * class's func_800C72D8. */
void func_800831C4(Unk80083108 *this, Unk80075EE4 *sum) {
    Entity *child = allocEntityMemory(0x38);

    if (child != NULL) {
        func_800833F8(entityOperatorNewAt(0x38, child), &this->base.pos, 0x180, 0xC0, this->base.unkBC);
    }
    entityAddSibling(&this->base.parts.entity, child);
    func_800C72D8(&this->base, sum);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80083248);

/* The entity class of virtual table D_800683A4 (constructor func_800833F8). */
typedef struct Unk800833F8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 vramY;
} Unk800833F8;

extern EntityVtable D_800683A4;

/* Constructs the entity of virtual table D_800683A4 at position, with arg3 at 0x2C, arg2 at 0x30 and
 * vramY. The match depends on reading the three words before storing any, as vecCopy does. */
Unk800833F8 *func_800833F8(Unk800833F8 *this, Vec3 *position, s32 arg2, s32 arg3, s32 vramY) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800683A4;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = arg3;
    this->unk30 = arg2;
    this->vramY = vramY;
    return this;
}

/* The update of D_800683A4's entity: 0x28 grows by 16 a frame, and the entity deletes itself once it reaches
 * 0x2C. */
void func_80083488(Unk800833F8 *this) {
    this->unk28 += 16;
    if (this->unk28 >= this->unk2C) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800834E4);

/* The entity class of virtual table D_8006837C (constructor func_80083700). */
typedef struct Unk80083700 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ VECTOR position; /* the character's node 2 (func_8007A4A4) */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48; /* func_8007768C of the character */
} Unk80083700;

extern EntityVtable D_8006837C;
/* words read as a vector (func_80083700) and as two words (func_800C32E0); in the main executable */
extern u32 D_80010814[];

/* Constructs the entity of virtual table D_8006837C for chara, at its node 2. */
Unk80083700 *func_80083700(Unk80083700 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006837C;
    this->chara = chara;
    func_8007A4A4(chara, &this->position, 2, (VECTOR *)D_80010814);
    this->unk2C = 0x80;
    this->unk30 = 0x180;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = 0;
    this->unk48 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80083790);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800838A8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008391C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80083B90);

/* The destructor of the entity class of vtable D_8006843C, which func_800827EC installs. */
void func_80083D4C(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_80068414, which func_80082EE8 installs. */
void func_80083D94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_80083DDC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800683A4, which func_800833F8 installs. */
void func_80083E7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006837C, which func_80083700 installs. */
void func_80083EC4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of virtual table D_80068784, derived from func_80076F34's. */
Unk80083F0C *func_80083F0C(Unk80083F0C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80068784;
    this->unk4A4 = 0;
    this->unk4A6 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068784, which func_80083F0C installs. */
void func_80083F48(Entity *this, s32 flags) {
    this->vtable = &D_80068784;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80083F9C);

/* Overrides func_8007B034 (entry 0x58 of Unk80076F34's vtable) in D_80068784: calls it,
 * then func_8007A6FC. */
void func_80083FA4(Unk80076F34 *this, s32 arg) {
    func_8007B034(this, arg);
    func_8007A6FC(this, -1, 160, 4);
}

/* Overrides func_8007B0E0 (entry 0x60 of Unk80076F34's vtable) in D_80068784: calls it,
 * then func_8007A6FC. */
void func_80083FDC(Unk80076F34 *this, s32 arg) {
    func_8007B0E0(this, arg);
    func_8007A6FC(this, -1, 160, 4);
}

/* Overrides func_8007B884: calls it, then func_8007A6FC(-1, 160, 2) every fourth step below 18. */
void func_80084014(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg < 18 && (arg & 3) == 0) {
        func_8007A6FC(this, -1, 160, 2);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80084068);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80084200);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008439C);

void func_80084C74(Unk80076F34 *this);
void func_80084D00(Unk80076F34 *this);

/* Entry 0xE0 of D_80068784 (func_8007B9FC's slot): func_80079AE4; at step 0 the entities of func_80084C74 and
 * func_80084D00, clip 24, func_8007A0E4 with 12, the words at 0x44C and sound 0x80; at step 5 unk3B4.vy
 * falls by unk3C4 * 0x6AA4 (16.16); below step 22 a rising unk3A4 slows unk3B4. */
void func_80084558(Unk80076F34 *this, s32 step) {
    func_80079AE4(this);
    switch (step) {
    case 0:
        func_80084C74(this);
        func_80084D00(this);
        func_80079830(this, 24, 0, 0, -1);
        this->unk3A4.vy = 0;
        func_8007A0E4(this, 12);
        this->unk44C = 0x45AC2A00;
        this->unk454 = func_80079E10(this) * 5460;
        this->unk458 = -0x6388;
        this->unk45C = 0;
        this->unk450 = 3;
        func_800784F4(this, 0x80);
        break;
    case 5:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x6AA4) >> 16;
        break;
    }
    if (step < 22 && this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80084694);

/* Entry 0x118 of D_80068784 (func_8007BC1C's slot): func_80079AE4; at step 0 the entities of func_80084C74 and func_80084D00, clip 25,
 * unk3B4.vy falls by unk3C4 * 0x38E0 (16.16), func_8007A0E4 with 12, the words at 0x44C and sound 0x80; at step 5
 * the same fall; below step 12 a rising unk3A4 slows unk3B4. */
void func_80084838(Unk80076F34 *this, s32 step) {
    func_80079AE4(this);
    switch (step) {
    case 0:
        func_80084C74(this);
        func_80084D00(this);
        func_80079830(this, 25, 0, 0, -1);
        this->unk3A4.vy = 0;
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x38E0) >> 16;
        func_8007A0E4(this, 12);
        this->unk44C = 0x45AC2A00;
        this->unk454 = func_80079E10(this) * 5460;
        this->unk458 = -0x6388;
        this->unk45C = 0;
        func_800784F4(this, 0x80);
        break;
    case 5:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x38E0) >> 16;
        break;
    }
    if (step < 12 && this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008499C);

/* func_80084B4C's local table of two entries; GCC keeps a local array's initializer as an anonymous constant
 * and copies it whole (.rodata stays in asm). */
typedef struct Unk800685DC {
    Unk800C5B18Entry entries[2];
} Unk800685DC;

extern Unk800685DC D_800685DC;

/* Creates func_800C5680's entity (0x690 bytes) for the character with a copy of D_800685DC, 2, 8 and arg, and
 * hands it to func_8007AD4C. */
void func_80084B4C(Unk80076F34 *this, s32 arg) {
    Unk800685DC table;
    Entity *child;

    table = D_800685DC;
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, table.entries, 2, 8, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_8008514C's class (0x48 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_80084C04(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_8008514C(entityOperatorNewAt(0x48, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800856D0's entity (0x34 bytes) at the character's position, and hands it to
 * func_8007AD4C. */
void func_80084C74(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Vec3 position;
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        func_800856D0(entityOperatorNewAt(0x34, child), this, func_800775D8(this, &position), obj);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80084D00);

/* Creates an entity of func_80085B7C's class (0xDC bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_80084E58(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0xDC);
    if (child != NULL) {
        func_80085B7C(entityOperatorNewAt(0xDC, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800862C0's class (0x54 bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_80084EC0(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800862C0(entityOperatorNewAt(0x54, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800868EC's entity (0xB8 bytes) at the character's point 18 moved by D_80068654 and 256
 * toward func_80079E10's side, standing still with arg in the velocity's pad; hands it to func_8007AD4C. */
void func_80084F28(Unk80076F34 *this, s16 arg) {
    Unk8006ED20 *obj;
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    obj = func_80077580(this);
    func_8007A4A4(this, (VECTOR *)&position, 18, &D_80068654);
    position.vx += func_80079E10(this) * 256;
    velocity.vx = 0;
    velocity.vy = 0;
    velocity.vz = 0;
    velocity.pad = arg;
    child = allocEntityMemory(0xB8);
    if (child != NULL) {
        func_800868EC(entityOperatorNewAt(0xB8, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80084FE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008514C);

/* The destructor of the entity class of vtable D_8006875C, which func_8008514C installs. */
void func_800852C0(Entity *this, s32 flags) {
    this->vtable = &D_8006875C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80085314);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800854B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800856D0);

/* The destructor of the entity class of vtable D_80068734, which func_800856D0 installs. */
void func_80085760(Entity *this, s32 flags) {
    this->vtable = &D_80068734;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800857B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80085848);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80085B7C);

/* The destructor of the entity class of vtable D_8006870C, which func_80085B7C installs. */
void func_80085C90(Entity *this, s32 flags) {
    this->vtable = &D_8006870C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80085CE4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80085EF4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800862C0);

/* The destructor of the entity class of vtable D_800686E4, which func_800862C0 installs. */
void func_8008634C(Entity *this, s32 flags) {
    this->vtable = &D_800686E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800863A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80086530);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800868EC);

/* The destructor of the UnkPartsEntity38 class of vtables D_800686BC and D_8006869C, which
 * func_800868EC installs. */
void func_80086A80(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006869C;
    this->entity.vtable = &D_800686BC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80086B20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80086CB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80086E48);

/* Constructs an entity of the class of vtable D_800689F4, which derives from Unk80076F34 and
 * gives it the table D_8006894C. */
Unk800870F0 *func_800870F0(Unk800870F0 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_800689F4;
    func_80079B24(&this->base, D_8006894C);
    this->unk4A4 = 0;
    return this;
}

/* The override in virtual table D_800689F4: func_80079B2C with 0. */
void func_80087138(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

/* A virtual function of Unk80076F34: func_8007B978; before step 16, func_8007835C and copies unk3A4 and unk3B4
 * to the Unk80076F34 of func_8007AEEC; at step 0, func_80079274 and func_800796F0 on it; at step 16, launches it
 * up and away from func_80079E10's side with newLaunch. */
void func_80087158(Unk80076F34 *this, s32 step) {
    Unk80076F34 *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    func_8007B978(this, step);
    other = func_8007AEEC(this);
    if (step < 16) {
        func_8007835C(this, 0);
        /* The match depends on each vector read into x, y and z before it is stored. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        other->unk3A4.vx = x;
        other->unk3A4.vy = y;
        other->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        other->unk3B4.vx = x;
        other->unk3B4.vy = y;
        other->unk3B4.vz = z;
    }
    switch (step) {
    case 0:
        func_80079274(other, 16);
        func_800796F0(other, 0, 0, 0);
        break;
    case 16:
        side = -func_80079E10(this);
        vel.vx = side * 10920;
        vel.vy = -18200;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_800689F4: func_8007B9C8, sets bit 6 of
 * 0x37C, then for arg 0 plays sound 0x85, and for arg 14 plays sound 0x82 and calls func_80087400. */
void func_800872A8(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x85);
        func_80079B2C(this, 1);
        break;
    case 14:
        func_80078540(this, 0x82);
        func_80079B2C(this, 0);
        func_80087400(this);
        break;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_800689F4: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 0 plays sound 0x85, arg 14 plays sound 0x82 and calls func_80087400; below 15 the vector at
 * 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_80087338(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x85);
        func_80079B2C(this, 1);
        break;
    case 14:
        func_80078540(this, 0x82);
        func_80079B2C(this, 0);
        func_80087400(this);
        break;
    }
    if (arg < 15) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Spawns the entity of func_80087690 0xC0 above the character (func_800775D8), with velocity
 * (96 forward, 0, 0). */
void func_80087400(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0xC0;
    velocity.vx = func_80079E10(this) * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_80087690(entityOperatorNewAt(0xC0, child), this, &position, &velocity);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80087498);

/* Creates an entity of func_8008787C's class (0x54 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_80087634(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_8008787C(entityOperatorNewAt(0x54, child), this);
    }
    func_8007AD4C(this, child);
}

/* Constructs func_800C7024's entity with 0x60, 0x6A81E00, 0x900, 3, then the virtual tables
 * D_800689CC (and D_800689AC for the base at 0x18). */
Unk800C7024 *func_80087690(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C7024(this, chara, position, velocity, 0x60, 0x6A81E00, 0x900, 3);
    this->parts.base.vtable = D_800689AC;
    this->parts.entity.vtable = &D_800689CC;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800876F4);

/* Constructs an Unk8008787C for the character: 0x1C, 0x24 and 0x2C are 0x1000, 0x44 is 0x80. */
Unk8008787C *func_8008787C(Unk8008787C *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068984;
    this->chara = chara;
    this->unk1C = 0x1000;
    this->unk20 = 0;
    this->unk24 = 0x1000;
    this->unk28 = 0;
    this->unk2C = 0x1000;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = 0x80;
    this->unk48 = 0;
    this->unk4C = 0;
    this->unk50 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008790C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80087C5C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80087E14);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80087EC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80087F3C);

/* The destructor of the entity class of vtable D_800689F4, which func_800870F0 installs. */
void func_80088094(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800880DC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_80068984, which func_8008787C installs. */
void func_8008817C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006895C, which func_80087E14 installs. */
void func_800881C4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of vtable D_80068C3C, which derives from Unk80076F34 and
 * gives it the table D_80068B6C. */
Unk80076F34 *func_8008820C(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_80068C3C;
    func_80079B24(this, D_80068B6C);
    return this;
}

/* The destructor of the entity class of vtable D_80068C3C, which func_8008820C installs. */
void func_80088250(Entity *this, s32 flags) {
    this->vtable = &D_80068C3C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The override of Unk80076F34's method at 0xA8 for virtual table D_80068C3C:
 * func_8007B70C, then for arg 1 to 4 func_8007835C with 9100 times the facing. */
void func_800882A4(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg >= 1 && arg <= 4) {
        func_8007835C(this, func_80079E10(this) * 9100);
    }
}

/* The override of Unk80076F34's method at 0xB0 for virtual table D_80068C3C:
 * func_8007B784, then for arg 16 to 19 func_8007835C with 18200 times the facing. */
void func_80088310(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg >= 16 && arg <= 19) {
        func_8007835C(this, func_80079E10(this) * 18200);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008837C);

void func_80088A64(Unk80076F34 *this, s32 arg);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_80068C3C: on 0, func_80079830,
 * func_80079B0C(27) and sound 0x8B; on 14, func_80088A64. */
void func_800884FC(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_80079B0C(this, 27);
        func_800784F4(this, 0x8B);
        break;
    case 14:
        func_80088A64(this, 0);
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088578);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008876C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088898);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088A64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088B1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088C24);

/* The destructor of the UnkPartsEntity38 class of vtables D_80068C14 and D_80068BF4, which
 * func_80088C24 installs. */
void func_80088E48(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068BF4;
    this->entity.vtable = &D_80068C14;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80088EE8);

/* The UnkPartsEntity38 class func_80088C24 constructs (virtual tables D_80068C14 and D_80068BF4), as far as
 * func_800890A4 needs it. */
typedef struct Unk80088C24 {
    /* 0x000 */ UnkPartsEntity38 base;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0x9C - 0x84];
    /* 0x09C */ Vec3 unk9C; /* the first of a run of positions */
    /* 0x0A8 */ u8 unkA8[0x100 - 0xA8];
    /* 0x100 */ Vec3 unk100; /* a constructor argument */
    /* 0x10C */ u8 unk10C[0x110 - 0x10C];
    /* 0x110 */ SVECTOR corners[4];
    /* 0x130 */ s32 unk130; /* the length of unk100 */
} Unk80088C24;

Entity *func_800C75FC(Entity *target, Vec3 *arg1, s32 arg2);

/* The second base's first method for virtual table D_80068BF4: func_8007BDF0 with sum; when func_80078878
 * finds a target in sum, func_800C75FC's entity on it; then a burst of 6 (func_800C62EC) at 0x9C, and the
 * entity deletes itself. */
void func_800890A4(Unk80088C24 *this, Unk80075EE4 *sum) {
    Entity *target;
    Entity *child;

    func_8007BDF0(&this->base.base, sum);
    target = func_80078878(sum);
    if (target != NULL) {
        entityAddSibling(&this->base.entity, func_800C75FC(target, &this->unk100, 0x80));
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unk9C, this->unk130 >> 2, 0x60, 6, 0x808080);
    }
    entityAddSibling(&this->base.entity, child);
    entityDelete(&this->base.entity);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80089180);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800893BC);

/* The destructor of the UnkPartsEntity38 class of vtables D_80068BCC and D_80068BAC, which
 * func_800893BC installs. */
void func_800895D0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068BAC;
    this->entity.vtable = &D_80068BCC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80089670);

/* Sets bit 0 of 0x44. */
void func_800899A0(Unk800AD5BC *this) {
    this->unk44 |= 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800899B4);

/* Constructs an entity of the class of virtual table D_80068EDC, derived from func_80076F34's. */
Unk80089FB8 *func_80089FB8(Unk80089FB8 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80068EDC;
    this->unk4A4 = 0;
    this->unk4A6 = 0;
    this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068EDC, which func_80089FB8 installs. */
void func_80089FF8(Entity *this, s32 flags) {
    this->vtable = &D_80068EDC;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008A04C);

void func_8008A44C(Unk80076F34 *this);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_80068EDC: func_8007B9C8; arg 1 plays
 * sound 0x92 and calls func_8008A44C, arg 6 calls func_8008A384 and plays sound 0x8D, and below 7 the x
 * at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_8008A218(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800784F4(this, 0x92);
        func_8008A44C(this);
    }
    if (arg == 6) {
        func_8008A384((Unk80089FB8 *)this);
        func_80078540(this, 0x8D);
    }
    if (arg < 7) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_80068EDC: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 1 plays sound 0x92 and calls func_8008A44C; arg 6 calls func_8008A384, plays sound 0x8D and pushes vx at 0x3B4 back by
 * 0x3000; below 7 the vector at 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_8008A2B4(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800784F4(this, 0x92);
        func_8008A44C(this);
    }
    if (arg == 6) {
        func_8008A384((Unk80089FB8 *)this);
        func_80078540(this, 0x8D);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 7) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_8008A9BC's entity (0x2CC bytes) at the character's point 12, flying 96 a frame toward
 * func_80079E10's side (kept at 0x4A4), hands it to func_8007AD4C and calls func_80079B0C(30). */
void func_8008A384(Unk80089FB8 *this) {
    Vec3 position;
    VECTOR velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(&this->base, (VECTOR *)&position, 12, &D_80010824);
    this->unk4A4 = func_80079E10(&this->base);
    velocity.vx = this->unk4A4 * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0x2CC);
    if (child != NULL) {
        func_8008A9BC(entityOperatorNewAt(0x2CC, child), &this->base, &position, &velocity, obj);
    }
    func_8007AD4C(&this->base, child);
    func_80079B0C(&this->base, 30);
}

/* Creates an entity of func_8008B124's class (0x48 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8008A44C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_8008B124(entityOperatorNewAt(0x48, child), this);
    }
    func_8007AD4C(this, child);
}

void func_80079B14(Unk80076F34 *this, s32 arg);
void func_8008A628(Unk80089FB8 *this, s32 angle);

/* A virtual function of Unk80089FB8: func_8007B9FC; at step 0 clears 0x4A8, func_80079B14(60) and sounds 0x93
 * and 0x8E; at steps 3 to 25 func_8008A628 turned to the facing (0 or 0x800) and the position at 0x3B4 moves
 * back by a sixteenth of the x at 0x3A4. */
void func_8008A4A8(Unk80089FB8 *this, s32 step) {
    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        func_800784F4(&this->base, 0x93);
        func_80078540(&this->base, 0x8E);
    }
    if (step >= 3 && step < 26) {
        if (func_80079E10(&this->base) > 0) {
            func_8008A628(this, 0);
        } else {
            func_8008A628(this, 0x800);
        }
        this->base.unk3B4.vx += -this->base.unk3A4.vx >> 4;
    }
}

/* func_8008A4A8 on func_8007BC1C, below step 26, and the position at 0x3B4 also moves back by the y at 0x3A8
 * when that is positive. */
void func_8008A560(Unk80089FB8 *this, s32 step) {
    func_8007BC1C(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        func_800784F4(&this->base, 0x93);
        func_80078540(&this->base, 0x8E);
    }
    if (step < 26) {
        if (func_80079E10(&this->base) > 0) {
            func_8008A628(this, 0);
        } else {
            func_8008A628(this, 0x800);
        }
        this->base.unk3B4.vx -= this->base.unk3A4.vx >> 4;
        if (this->base.unk3A4.vy > 0) {
            this->base.unk3B4.vy -= this->base.unk3A4.vy;
        }
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008A628);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008A730);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008A91C);

/* The entity func_8008A384 spawns (virtual tables D_80068EB4, and D_80068E94 for its base at 0x18). */
typedef struct Unk8008A9BC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ Vec3 position;
    /* 0x090 */ u8 unk90[0x2A4 - 0x90];
    /* 0x2A4 */ Vec3 velocity;
    /* 0x2B0 */ u8 unk2B0[0x2B4 - 0x2B0];
    /* 0x2B4 */ s32 unk2B4;
    /* 0x2B8 */ s32 unk2B8;
    /* 0x2BC */ s32 speed; /* the velocity's length */
    /* 0x2C0 */ s32 facing; /* the character's (func_80079E10) */
    /* 0x2C4 */ s32 unk2C4;
    /* 0x2C8 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008A9BC;

/* Constructs func_8008A384's entity for the character at position, flying at velocity, and adds its member
 * at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk8008A9BC *func_8008A9BC(Unk8008A9BC *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x6A81800, (Vec3 *)velocity, 2);
    this->unk18.vtable = D_80068E94;
    this->entity.vtable = &D_80068EB4;
    func_80075EE4(&this->unk38, position, 0xC0, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->unk2B4 = 0;
    this->unk2B8 = 0;
    this->speed = vecGetLength(velocity);
    this->facing = func_80079E10(this->chara);
    this->unk2C4 = 0;
    this->vramY = func_8007768C(chara);
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_80068EB4 and D_80068E94, which
 * func_8008A9BC installs. */
void func_8008AB44(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068E94;
    this->entity.vtable = &D_80068EB4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

extern EntityVtable D_8006D4D4;

/* The constructor of the entity class of vtable D_8006D4D4, which the game inlines: func_800C2FBC with frame 8,
 * no velocity (D_80010814), size 0x40, color 0xA0A0A0 and no flags. */
static inline void func_8006D4D4_init(Entity *this, VECTOR *pos) {
    func_800C2FBC((Unk800C2F14 *)this, 8, pos, (VECTOR *)D_80010814, 0x40, 0xA0A0A0, 0);
    this->vtable = &D_8006D4D4;
}

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes) at owner's position: NULL when the allocation
 * fails. */
static inline Entity *func_8008ABE4_newFlash(Unk8008A9BC *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of func_800C62EC's burst (0x3C bytes, kind 4, five particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *func_8008ABE4_newBurst(Unk8008A9BC *owner) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 4, &owner->position, 0x20, 0x100, 5, 0x808080);
    }
    return child;
}

/* The base at 0x18's first method for func_8008A9BC's class: func_8007BDF0 with sum; when sum holds an entity
 * of kind 0x10000 (func_8002BD68), sets bit 0 of the member at 0x38 and adds a flash at the position when
 * func_80078DE0 holds for it, else a burst of 5. */
void func_8008ABE4(Unk8008A9BC *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->unk18, sum);
    target = sum->unk14;
    if (target != NULL && func_8002BD68(target) == 0x10000) {
        this->unk38.flags |= 1;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, func_80078DE0((Unk80076F34 *)target) ? func_8008ABE4_newFlash(this)
                                                                             : func_8008ABE4_newBurst(this));
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008AD20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008ADC4);

/* The entity class of virtual table D_80068E6C (constructor func_8008B124). */
typedef struct Unk8008B124 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position; /* the character's (func_800775D8) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 unk44; /* func_8007768C of the character */
} Unk8008B124;

/* Constructs the entity of virtual table D_80068E6C for chara, at its position. */
Unk8008B124 *func_8008B124(Unk8008B124 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068E6C;
    this->chara = chara;
    func_800775D8(chara, &this->position);
    this->unk2C = 0x40;
    this->unk30 = 0x40;
    this->unk34 = 0x80;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068E6C, which func_8008B124 installs. */
void func_8008B1A4(Entity *this, s32 flags) {
    this->vtable = &D_80068E6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008B1F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008B25C);

/* The entity class of virtual table D_80068DAC (constructor func_8008B62C, made by func_8008A91C). */
typedef struct Unk8008B62C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x30 - 0x28];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s16 unk34; /* func_8007768C of the character */
} Unk8008B62C;

/* Constructs the entity of virtual table D_80068DAC for chara at position. The match depends on reading
 * the three words before storing any, as vecCopy does, and on storing chara between the reads and the
 * writes. */
Unk8008B62C *func_8008B62C(Unk8008B62C *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DAC;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->chara = chara;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk30 = 0;
    this->unk34 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068DAC, which func_8008B62C installs. */
void func_8008B6A8(Entity *this, s32 flags) {
    this->vtable = &D_80068DAC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008B6FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008B860);

/* The entity of func_8008BE04 (vtable D_80068D84, 0x34 bytes): a character's, with its 0x440 (func_8007768C). */
typedef struct Unk8008BE04 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ u8 unk1C[0x2C - 0x1C];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008BE04; /* size 0x34 */

/* Constructs the character's entity of vtable D_80068D84. */
Unk8008BE04 *func_8008BE04(Unk8008BE04 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068D84;
    this->chara = chara;
    this->unk2C = 0;
    this->vramY = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068D84, which func_8008BE04 installs. */
void func_8008BE5C(Entity *this, s32 flags) {
    this->vtable = &D_80068D84;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008BEB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008BF2C);

/* The class func_8008C20C constructs (virtual tables D_80068E44, and D_80068E24 for its base at 0x18) for an
 * Unk80089FB8 character: like Unk800ACA60, with a flag at 0xBA. */
typedef struct Unk8008C20C {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80089FB8 *owner;
    /* 0x84 */ Unk8006ED20 *list; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 vel;
    /* 0xA4 */ u8 unkA4[0xB4 - 0xA4];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s16 unkB8; /* func_8007768C of the owner */
    /* 0xBA */ s16 unkBA; /* set, with the part's flag 1, when made while the owner's 0x4A8 is set */
} Unk8008C20C;

/* Constructs Unk8008C20C for chara at position, moving by velocity (its base gets it times 4), adds the part
 * at 0x38 to one of list's two lists (func_80077574 picks which), and flags it if chara's 0x4A8 is set. */
Unk8008C20C *func_8008C20C(Unk8008C20C *this, Unk80089FB8 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, &chara->base, 0x2AC0D00, (Vec3 *)velocity, 0);
    this->parts.base.vtable = D_80068E24;
    this->parts.entity.vtable = &D_80068E44;
    func_80075EE4(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 2;
    v.vy <<= 2;
    v.vz <<= 2;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkB4 = 0;
    this->unkBA = 0;
    this->unkB8 = func_8007768C(&chara->base);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(&this->owner->base) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    if (this->owner->unk4A8 != 0) {
        this->unkBA = 1;
        this->parts.part.flags |= 1;
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_80068E44 and D_80068E24, which
 * func_8008C20C installs. */
void func_8008C3A0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068E24;
    this->entity.vtable = &D_80068E44;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008C440);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008C620);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008C748);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008C94C);

/* The destructor of the entity class of vtable D_80068DFC, which func_8008C94C installs. */
void func_8008C9AC(Entity *this, s32 flags) {
    this->vtable = &D_80068DFC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008CA00);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008CA74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008CE68);

/* The destructor of the entity class of vtable D_80068DD4, which func_8008CE68 installs. */
void func_8008CF18(Entity *this, s32 flags) {
    this->vtable = &D_80068DD4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008CF6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D028);

/* The character class func_8008D21C constructs (virtual table D_8006938C); func_80091A44's derives
 * from it. */
typedef struct Unk8008D21C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ s16 unk4A6;
    /* 0x4A8 */ s16 unk4A8;
    /* 0x4AA */ s16 unk4AA;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
    /* 0x4C4 */ s32 unk4C4;
    /* 0x4C8 */ s32 unk4C8;
    /* 0x4CC */ s32 unk4CC;
} Unk8008D21C;

/* Constructs an object of the entity class of vtable D_8006938C, which derives from func_80076F34's,
 * its fields from 0x4A6 cleared. */
Unk8008D21C *func_8008D21C(Unk8008D21C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006938C;
    this->unk4A6 = 0;
    this->unk4A8 = 0;
    this->unk4AA = 0;
    this->unk4B0 = 0;
    this->unk4B4 = 0;
    this->unk4B8 = 0;
    this->unk4BC = 0;
    this->unk4C0 = 0;
    this->unk4C4 = 0;
    this->unk4C8 = 0;
    this->unk4CC = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006938C, which func_8008D21C installs. */
void func_8008D27C(Entity *this, s32 flags) {
    this->vtable = &D_8006938C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D2D0);

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it,
 * then func_8007A6FC when arg is 17. */
void func_8008D31C(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 17) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D368);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D584);

/* An override of Unk80076F34's method at 0xA8 (virtual tables D_8006938C and
 * D_80069564): func_8007B70C, then func_8008E900 when arg is 8. */
void func_8008D650(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 8) {
        func_8008E900(this);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it,
 * then func_8007A6FC when arg is 22. */
void func_8008D694(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 22) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual tables D_8006938C and D_80069564: for arg 0
 * starts clip 0x16, func_8008E5C4(0), sets 0x334 to 0x1E and plays sound 0x9A (0xA3 unless 0x218 is 5);
 * for arg 10 spawns func_8008E51C's entity. */
void func_8008D6E0(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_8008E5C4(this, 0);
        func_80079B0C(this, 0x1E);
        if (func_80077568(this) == 5) {
            func_800784F4(this, 0x9A);
        } else {
            func_800784F4(this, 0xA3);
        }
        break;
    case 10:
        func_8008E51C(this);
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D788);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008D97C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008DB98);

/* The override of Unk80076F34's method at 0x110 for virtual tables D_8006938C and D_80069564: for arg 0
 * starts clip 0x17, func_8008E5C4(1), sets 0x334 to 0x14 and plays sound 0x9A (0xA3 unless 0x218 is 5);
 * for arg 10 spawns func_8008E51C's entity. */
void func_8008DC24(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x17, 2, 0, -1);
        func_8008E5C4(this, 1);
        func_80079B0C(this, 0x14);
        if (func_80077568(this) == 5) {
            func_800784F4(this, 0x9A);
        } else {
            func_800784F4(this, 0xA3);
        }
        break;
    case 10:
        func_8008E51C(this);
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008DCCC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008DD50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008DE84);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008E48C);

/* Spawns the entity of func_8008EAB4 at an offset from node 0xC of the
 * character's model, with velocity (64 forward, 0, 0). */
void func_8008E51C(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_8007A4A4(this, (VECTOR *)&position, 0xC, &D_800690D0);
    velocity.vx = func_80079E10(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        func_8008EAB4(entityOperatorNewAt(0x160, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_8008F474's class (0x150 bytes) at the owner's point 12 moved by D_800690E0,
 * with arg in the vector's pad, and hands it to func_8007AD4C. */
void func_8008E5C4(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR pos;
    Entity *child;

    func_8007A4A4(this, &pos, 12, &D_800690E0);
    pos.pad = arg;
    child = allocEntityMemory(0x150);
    if (child != NULL) {
        func_8008F474(entityOperatorNewAt(0x150, child), this, &pos, obj);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008E658);

/* Creates an entity of func_800C5680's class (0x690 bytes) with g++'s new and the table
 * D_800690F0, and hands it to func_8007AD4C. */
void func_8008E70C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_800690F0, 3, 8, 14);
    }
    func_8007AD4C(this, child);
}

/* Spawns the entity of func_800902E0 at the character (func_800775D8), with
 * velocity (256 forward, -64, 0). */
void func_8008E784(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = func_80079E10(this) * 256;
    velocity.vy = -64;
    velocity.vz = 0;
    child = allocEntityMemory(0x920);
    if (child != NULL) {
        func_800902E0(entityOperatorNewAt(0x920, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008E824);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008E900);

/* Constructs an Unk8008E9B4 (vtable D_80069364) for owner: func_800C3CFC with pos, arg4 and 0xFFC080,
 * then 6 in unk50. */
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Unk80076F34 *owner, s32 arg2, Vec3 *pos, s32 arg4) {
    func_800C3CFC(&this->base, pos, arg4, 0xFFC080);
    /* The match depends on this order: vtable, unk54, unk50. */
    this->base.entity.vtable = &D_80069364;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

/* The gather of the Unk8008E9B4 of D_80069364: func_800C3E64, then puts the world position of the
 * owner's point 12 (func_8007A4A4) at 0x28. */
void func_8008EA10(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    func_8007A4A4(this->unk54, &this->base.unk28, 12, &origin);
}

/* Constructs an Unk8008E9B4 with vtable D_8006933C (see func_8008E9B4). */
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Unk80076F34 *owner, s32 arg2, Vec3 *pos, s32 arg4) {
    func_800C3CFC(&this->base, pos, arg4, 0xFFC080);
    /* The match depends on this order, as in func_8008E9B4. */
    this->base.entity.vtable = &D_8006933C;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008EAB4);

/* The destructor of the UnkPartsEntity38 class of vtables D_80069314 and D_800692F4, which
 * func_8008EAB4 installs. */
void func_8008ED14(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_800692F4;
    this->entity.vtable = &D_80069314;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008EDB4);

/* The second base's first method for virtual table D_800692F4: func_8007BDF0 with sum; when func_80078878
 * finds a character in sum, takes its position (unused) and sets bit 0 of the part's flags. */
void func_8008EFDC(UnkPartsEntity38 *this, Unk80075EE4 *sum) {
    Entity *chara;
    Vec3 position;

    func_8007BDF0(&this->base, sum);
    chara = func_80078878(sum);
    if (chara != NULL) {
        func_800775D8((Unk80076F34 *)chara, &position);
        this->part.flags |= 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008F038);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008F474);

/* The destructor of the entity class of vtable D_800692CC, which func_8008F474 installs. */
void func_8008F64C(Entity *this, s32 flags) {
    this->vtable = &D_800692CC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008F6A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008F7F8);

/* The direction (ratan2) from the centre at 0x38 to point index, on x and y. */
s16 func_8008F9B0(Unk8008F474 *this, s32 index) {
    return ratan2(this->unk48[index].vx - this->unk38.vx, this->unk48[index].vy - this->unk38.vy);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008F9F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008FD18);

/* The destructor of the UnkPartsEntity38 class of vtables D_800692A4 and D_80069284, which
 * func_8008FD18 installs. */
void func_8008FEF0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80069284;
    this->entity.vtable = &D_800692A4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8008FF90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80090074);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009013C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800902E0);

/* The destructor of the entity class of vtable D_8006925C, which func_800902E0 installs. */
void func_8009050C(Entity *this, s32 flags) {
    this->vtable = &D_8006925C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80090560);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800908E8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80090F04);

/* The destructor of the UnkPartsEntity38 class of vtables D_80069234 and D_80069214, which
 * func_80090F04 installs. */
void func_80091060(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80069214;
    this->entity.vtable = &D_80069234;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091100);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800911EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091328);

/* The entity class of vtable D_800691EC (0x38 bytes), made for a character 0x40 below a position. */
typedef struct Unk80091498 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 unk22; /* the character's 0x440 (func_8007768C) */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ Vec3 position;
} Unk80091498;

/* Constructs the entity of vtable D_800691EC for a character, 0x40 below position. */
Unk80091498 *func_80091498(Unk80091498 *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800691EC;
    this->unk22 = func_8007768C(chara);
    this->chara = chara;
    this->unk1E = 0;
    this->unk20 = 0;
    this->facing = func_80079E10(chara);
    this->unk24 = 0x1C00;
    x = position->vx;
    y = position->vy + 0x40;
    z = position->vz;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The destructor of the entity class of vtable D_800691EC, which func_80091498 installs. */
void func_80091530(Entity *this, s32 flags) {
    this->vtable = &D_800691EC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091584);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091620);

/* The destructor of the entity class of vtable D_80069364, which func_8008E9B4 installs. */
void func_80091908(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006933C, which func_8008EA58 installs. */
void func_80091950(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4FC, which func_800C32E0
 * installs: game has 2 identical ones. */
void func_80091998(Entity *this, s32 flags) {
    this->vtable = &D_8006D4FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800919EC);

/* Constructs an object of the entity class of vtable D_80069564, which derives from func_8008D21C's. */
Unk8008D21C *func_80091A44(Unk8008D21C *this) {
    func_8008D21C(this);
    this->base.entity.vtable = &D_80069564;
    return this;
}

/* The destructor of the entity class of vtable D_80069564, which func_80091A44 installs. */
void func_80091A78(Entity *this, s32 flags) {
    this->vtable = &D_80069564;
    func_8008D27C(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* For arg 1, creates func_800925AC's entity (0x74 bytes) at the character's position and hands it to
 * func_8007AD4C. */
void func_80091ACC(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Entity *child;

    if (arg == 1) {
        func_800775D8(this, &position);
        child = allocEntityMemory(0x74);
        if (child != NULL) {
            func_800925AC(entityOperatorNewAt(0x74, child), this, &position);
        }
        func_8007AD4C(this, child);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091B3C);

/* Spawns the entity of func_80091E60 at the character (func_800775D8), with
 * velocity (64 forward, 0, 0). */
void func_80091D4C(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = func_80079E10(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xF0);
    if (child != NULL) {
        func_80091E60(entityOperatorNewAt(0xF0, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800C5680's class (0x690 bytes) with g++'s new and the table
 * D_800694B4, and hands it to func_8007AD4C. */
void func_80091DE8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_800694B4, 2, 8, 15);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80091E60);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006953C and D_8006951C, which
 * func_80091E60 installs. */
void func_8009201C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006951C;
    this->entity.vtable = &D_8006953C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800920BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800921B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009230C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800925AC);

/* The destructor of the entity class of vtable D_800694F4, which func_800925AC installs. */
void func_800926A4(Entity *this, s32 flags) {
    this->vtable = &D_800694F4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800926F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800927C8);

/* The character class of virtual table D_8006983C (constructor func_80092A68). */
typedef struct Unk80092A68 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
} Unk80092A68;

extern Unk80079B24Entry D_800696DC[];

/* Constructs a character of the class of vtable D_8006983C (which derives from Unk80076F34) with the
 * table D_800696DC. */
Unk80092A68 *func_80092A68(Unk80092A68 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006983C;
    this->unk4A4 = 0;
    func_80079B24(&this->base, D_800696DC);
    return this;
}

/* The destructor of the entity class of vtable D_8006983C, which func_80092A68 installs. */
void func_80092AB0(Entity *this, s32 flags) {
    this->vtable = &D_8006983C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80092B04);

/* A method of the class of virtual table D_8006983C: func_80093144, which sets its field 0x4A8. */
void func_80092B90(void *this) {
    func_80093144(this);
}

/* For arg 15, creates func_80093AD8's entity (0x58 bytes) for the character and hands it to
 * func_8007AD4C. */
void func_80092BB0(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj;
    Entity *child;

    if (arg == 15) {
        obj = func_80077580(this);
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_80093AD8(entityOperatorNewAt(0x58, child), this, obj);
        }
        func_8007AD4C(this, child);
    }
}

/* The override of Unk80076F34's method at 0xB0 for virtual table D_8006983C: func_8007B784,
 * then for arg 5 pushes vx at 0x3B4 by field 0x8 of func_80077698's entry 2 times the scale at 0x3C4,
 * along the facing. */
void func_80092C2C(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 5) {
        s32 push = ((s64)func_80077698(this)->entries[2].unk8[0] * this->unk3C4) >> 16;

        this->unk3B4.vx += push * func_80079E10(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80092CBC);

/* Spawns the entity of func_80093274 0x84 above the character (func_800775D8), with velocity
 * (24 forward, 0, 0). */
void func_80092DCC(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x84;
    velocity.vx = func_80079E10(this) * 24;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_80093274(entityOperatorNewAt(0x58, child), this, &position, &velocity);
    }
    func_8007AD4C(this, child);
}

/* Overrides func_8007B9C8: calls it, plays sound 0xAC at step 0 and spawns func_80092F58's entity
 * at step 10. */
void func_80092E64(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xAC);
    }
    if (arg == 10) {
        func_80092F58(this);
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006983C: func_8007BBE8; arg 0 plays
 * sounds 0xAC and 0xA6, arg 14 calls func_80092F58, and below 15 the position at 0x3B4 moves back by a
 * sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_80092EBC(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xAC);
        func_80078540(this, 0xA6);
    }
    if (arg == 14) {
        func_80092F58(this);
    }
    if (arg < 15) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_80093460's entity (0xD4 bytes) 0x84 above the character, moving 144 a frame toward
 * func_80079E10's side; hands it to func_8007AD4C and plays sound 0xA7. */
void func_80092F58(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x84;
    velocity.vx = func_80079E10(this) * 144;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        func_80093460(entityOperatorNewAt(0xD4, child), this, obj, &position, &velocity);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xA7);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093018);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093144);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093150);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800931B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093274);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800932F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093340);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093460);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093514);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800935E8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093640);

/* Constructs an entity of D_800697A4: func_800C2FBC with entry 9 of D_8006D5C4, 0x19 and 0x202080. */
Entity *func_800938A4(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    func_800C2FBC((Unk800C2F14 *)this, 9, pos, vel, 0x19, 0x202080, 0);
    this->vtable = &D_800697A4;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800938F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800939B0);

/* Constructs an Unk80093AD8 for the character, its fields 0. obj is not read (func_80092BB0 passes it). */
Unk80093AD8 *func_80093AD8(Unk80093AD8 *this, Unk80076F34 *chara, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006977C;
    this->chara = chara;
    this->unk1C[0] = 0;
    this->unk1C[1] = 0;
    this->unk1C[2] = 0;
    this->unk1C[3] = 0;
    this->unk1C[4] = 0;
    this->unk1C[5] = 0;
    this->unk1C[6] = 0;
    this->unk1C[7] = 0;
    this->unk1C[8] = 0;
    /* the game clears 0x40 after 0x48 and 0x4C */
    this->unk48 = 0;
    this->unk4C = 0;
    this->unk40 = 0;
    this->unk50 = 0;
    this->unk54 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_8006977C, which func_80093AD8 installs. */
void func_80093B60(Entity *this, s32 flags) {
    this->vtable = &D_8006977C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093BB4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80093E30);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009401C);

/* The destructor of the entity class of vtable D_80069754, which func_8009401C installs. */
void func_800940DC(Entity *this, s32 flags) {
    this->vtable = &D_80069754;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094130);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094258);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800942D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800945F8);

/* The destructor of the UnkPartsEntity3C class of vtable D_8006972C, which func_800945F8 installs. */
void func_800947EC(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006972C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094880);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094AF8);

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80094DC8(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_80094E1C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80094EBC(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of virtual table D_80069C04, derived from func_80076F34's. */
Unk80094F10 *func_80094F10(Unk80094F10 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80069C04;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069C04, which func_80094F10 installs. */
void func_80094F48(Entity *this, s32 flags) {
    this->vtable = &D_80069C04;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094F9C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80094FFC);

/* The override of Unk80076F34's method at 0xB8 for virtual table D_80069C04: func_8007B80C,
 * then for arg 4 pushes vx at 0x3B4 by field 0x8 of func_80077698's entry 4 times the scale at 0x3C4,
 * along the facing. */
void func_80095008(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 4) {
        s32 push = ((s64)func_80077698(this)->entries[4].unk8[0] * this->unk3C4) >> 16;

        this->unk3B4.vx += push * func_80079E10(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095098);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095294);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095330);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095480);

/* Creates func_800971E0's entity (0x30 bytes) for the character with arg, and hands it to
 * func_8007AD4C. */
void func_800955D4(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        func_800971E0(entityOperatorNewAt(0x30, child), this, arg, obj);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800962B8's class (0x15C bytes) at the owner's point 6 moved by D_80069964, moving
 * 192 a frame toward func_80079E10's side, hands it to func_8007AD4C and plays sound 0xAE. */
void func_80095654(Unk80076F34 *this) {
    VECTOR pos;
    VECTOR vel;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(this, &pos, 6, &D_80069964);
    vel.vx = func_80079E10(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0x15C);
    if (child != NULL) {
        func_800962B8(entityOperatorNewAt(0x15C, child), this, obj, &pos, &vel);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xAE);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095714);

/* Creates an entity of func_80096F6C's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800957C0(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_80096F6C(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

void func_80095894(Unk80076F34 *this);
void func_80095928(Unk80076F34 *this);

/* A virtual function of Unk80076F34: func_8007B9FC; at step 0 sound 0xB4, at step 7 func_80095894, at step 14
 * func_80095928 20 times. */
void func_8009581C(Unk80076F34 *this, s32 step) {
    s32 i;

    func_8007B9FC(this, step);
    if (step == 0) {
        func_800784F4(this, 0xB4);
    }
    if (step == 7) {
        func_80095894(this);
    }
    if (step == 14) {
        for (i = 0; i < 20; i++) {
            func_80095928(this);
        }
    }
}

/* Creates func_8009764C's entity (0xA4 bytes) facing func_80079E10's side, hands it to func_8007AD4C
 * and plays sound 0xAF. */
void func_80095894(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Entity *child = allocEntityMemory(0xA4);

    if (child != NULL) {
        func_8009764C(entityOperatorNewAt(0xA4, child), this, obj, func_80079E10(this));
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xAF);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095928);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095A70);

/* Creates an entity of func_80097FA0's class (0xB4 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_80095C44(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0xB4);

    if (child != NULL) {
        func_80097FA0(entityOperatorNewAt(0xB4, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Spawns the entity of func_80098A78 384 ahead of and 0x140 above the character, with velocity (192
 * forward, 0, 0), then plays sound 0xB0. */
void func_80095CB4(Unk80076F34 *this) {
    s32 facing = func_80079E10(this);
    Unk8006ED20 *unk228;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = facing * 192;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 384;
    position.vy -= 0x140;
    unk228 = func_80077580(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_80098A78(entityOperatorNewAt(0xC8, child), this, unk228, &position, &velocity);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xB0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095D88);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80095E8C);

/* The entity of D_80069BDC (func_80095294 makes it, 0x40 bytes): a character's effect at a position,
 * moved 15 down at each of its three gathers. */
typedef struct Unk80095F98 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *unk18; /* the character */
    /* 0x1C */ Vec3 unk1C;         /* a position */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C; /* the character's facing (func_80079E10) */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* 60, less 15 at each gather */
    /* 0x38 */ s32 unk38; /* gathers so far */
    /* 0x3C */ s16 unk3C; /* func_8007768C of the character */
} Unk80095F98;

/* Constructs the entity of D_80069BDC for a character at a position. */
Unk80095F98 *func_80095F98(Unk80095F98 *this, Unk80076F34 *chara, Vec3 *position, s32 facing, s32 arg4) {
    s32 vx, vy, vz;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069BDC;
    this->unk18 = chara;
    vx = position->vx;
    vy = position->vy;
    vz = position->vz;
    this->unk2C = facing;
    this->unk34 = 60;
    this->unk38 = 0;
    this->unk1C.vx = vx;
    this->unk1C.vy = vy;
    this->unk1C.vz = vz;
    this->unk30 = arg4;
    this->unk3C = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80069BDC, which func_80095F98 installs. */
void func_8009603C(Entity *this, s32 flags) {
    this->vtable = &D_80069BDC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096090);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096104);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800962B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096360);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096538);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800965D8);

/* The entity class whose method func_80096538 is (an Unk800C7024 with a value at 0xC0). */
typedef struct Unk80096984 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ s32 unkC0;
} Unk80096984;

void *func_800969E0(void *this, s32 arg);

/* Spawns the entity of func_800969E0 with the value at 0xC0, as a sibling. */
void func_80096984(Unk80096984 *this) {
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800969E0(entityOperatorNewAt(0x40, child), this->unkC0);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800969E0);

/* The destructor of the entity class of vtable D_80069B6C, which func_800969E0 installs. */
void func_80096A54(Entity *this, s32 flags) {
    this->vtable = &D_80069B6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096AA8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096B54);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096CFC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096DC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096E64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80096F6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097008);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800970A8);

/* The entity class of virtual table D_80069AF4 (constructor func_800971E0). */
typedef struct Unk800971E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 unk2C; /* func_8007768C of the character */
} Unk800971E0;

/* Constructs the entity of virtual table D_80069AF4 for chara with arg2 (obj, which its spawner passes, is
 * not read). */
Unk800971E0 *func_800971E0(Unk800971E0 *this, Unk80076F34 *chara, s32 arg2, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069AF4;
    this->chara = chara;
    this->unk1C = 0;
    this->unk20 = 0;
    /* The match depends on storing arg2 after 0x28 (it fills the call's delay slot). */
    this->unk28 = 0;
    this->unk24 = arg2;
    this->unk2C = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80069AF4, which func_800971E0 installs. */
void func_80097250(Entity *this, s32 flags) {
    this->vtable = &D_80069AF4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800972A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009733C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800975D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009764C);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069ACC, which func_8009764C installs. */
void func_80097790(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069ACC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097824);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097A30);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097D50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097DAC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097E90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80097FA0);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A7C, which func_80097FA0 installs. */
void func_80098128(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A7C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800981BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800983FC);

/* Creates an entity of func_80098720's class (0x44 bytes) with g++'s new, and hands it to entityAddSibling. */
void func_800986C0(Unk80097FA0 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_80098720(entityOperatorNewAt(0x44, child), this->unk84, &this->unk8C);
    }
    entityAddSibling(&this->parts.entity, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80098720);

/* The destructor of the entity class of vtable D_80069A54, which func_80098720 installs. */
void func_800987B8(Entity *this, s32 flags) {
    this->vtable = &D_80069A54;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009880C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800988A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80098A78);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A2C, which func_80098A78 installs. */
void func_80098C04(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A2C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80098C98);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80098ECC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009918C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009921C);

/* The destructor of the entity class of vtable D_80069A04, which func_8009921C installs. */
void func_800992F4(Entity *this, s32 flags) {
    this->vtable = &D_80069A04;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099348);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099410);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800995BC);

/* The destructor of the entity class of vtable D_800699DC, which func_800995BC installs. */
void func_80099680(Entity *this, s32 flags) {
    this->vtable = &D_800699DC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800996D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800997D0);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800998E0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80099980(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800999D4(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80099A28(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of virtual table D_8006A2AC, derived from func_80076F34's. */
Unk80099A7C *func_80099A7C(Unk80099A7C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006A2AC;
    this->unk4A8 = 0;
    this->unk4CC = 0;
    return this;
}

/* Stops the sound at 0x4CC. */
void func_80099AB8(Unk80099A7C *this) {
    stopSound(this->unk4CC);
    this->unk4CC = 0;
}

/* The override of Unk80076F34's method at 0xA8 for virtual table D_8006A2AC: sets bit 6 of 0x37C; arg 0
 * starts clip 6, arg 5 runs func_80077698's entry 1 from the start (func_8007B470), and
 * from 6 on runs it at arg. */
void func_80099AE8(Unk80076F34 *this, s32 arg) {
    Unk8007B470Entry *record;

    func_80079AE4(this);
    record = &func_80077698(this)->entries[1];
    switch (arg) {
    case 0:
        func_80079830(this, 6, 0, 0, -1);
        break;
    case 5:
        func_8007B470(this, 0, record, 1);
        break;
    }
    if (arg > 5) {
        func_8007B470(this, arg, record, 1);
    }
}

/* The override of Unk80076F34's method at 0xC0 for virtual table D_8006A2AC: sets bit 6 of 0x37C; arg 0
 * starts clip 8, arg 6 runs func_80077698's entry 3 from the start with 2 (func_8007B470), and
 * from 7 on runs it at arg. */
void func_80099B98(Unk80076F34 *this, s32 arg) {
    Unk8007B470Entry *record;

    func_80079AE4(this);
    record = &func_80077698(this)->entries[3];
    switch (arg) {
    case 0:
        func_80079830(this, 8, 2, 0, -1);
        break;
    case 6:
        func_8007B470(this, 0, record, 2);
        break;
    }
    if (arg > 6) {
        func_8007B470(this, arg, record, 1);
    }
}

/* Overrides func_8007BB64 (entry 0x10C of Unk80076F34's vtable) in D_8006A2AC: calls it, then
 * func_80099D18 when arg is 2; from 3 on, sets bits 0xD0000 of unk448 when it is not zero. */
void func_80099C48(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_80099D18(this);
    }
    if (arg >= 3 && this->unk448 != 0) {
        this->unk448 |= 0xD0000;
    }
}

/* Overrides func_8007BA98 (entry 0xFC of Unk80076F34's vtable) in D_8006A2AC: calls it, then
 * func_80099D18 when arg is 2; from 3 on, sets bits 0xD0000 of unk448 when it is not zero. */
void func_80099CB0(Unk80076F34 *this, s32 arg) {
    func_8007BA98(this, arg);
    if (arg == 2) {
        func_80099D18(this);
    }
    if (arg >= 3 && this->unk448 != 0) {
        this->unk448 |= 0xD0000;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099D18);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099DC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099F30);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_80099FBC);

/* Calls func_8007AE34 on the character at 0x4A8, if there is one, and clears it. */
void func_8009A0EC(Unk80099A7C *this) {
    if (this->unk4A8 != NULL) {
        func_8007AE34(this->unk4A8);
    }
    this->unk4A8 = NULL;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A128);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A1B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A290);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A35C);

/* Plays sound 0xB6 (its handle kept at 0x4CC), creates func_8009AB40's entity (0x4C bytes) for the
 * character's 0x440, hands it to func_8007AD4C and calls func_80079B0C(50). */
void func_8009A44C(Unk80099A7C *this) {
    Entity *child;

    this->unk4CC = func_80078540(&this->base, 0xB6);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        func_8009AB40(entityOperatorNewAt(0x4C, child), &this->base, func_8007768C(&this->base));
    }
    func_8007AD4C(&this->base, child);
    func_80079B0C(&this->base, 50);
}

/* Spawns the entity of func_8009B834 for the character (func_8007AD4C). */
void func_8009A4D8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x50);

    if (child != NULL) {
        func_8009B834(entityOperatorNewAt(0x50, child), this, 0x200, 0xFF8080);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A540);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A644);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A764);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A814);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009A920);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009AB40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009ABE0);

/* The update of the entities of D_8006A284: func_8009AFC4, func_8009B370, then func_8009B664. */
void func_8009AF7C(Entity *this, s32 arg) {
    func_8009AFC4(this, arg);
    func_8009B370(this, arg);
    func_8009B664(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009AFC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009B370);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009B664);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009B834);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009BA18);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009BC24);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009BE08);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009BE80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009BF6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C048);

/* The UnkPartsEntity38 class func_8009C294 constructs (virtual tables D_8006A1EC and D_8006A1CC). */
typedef struct Unk8009C294 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ u8 unk80[0xB4 - 0x80];
    /* 0xB4 */ s32 unkB4;
} Unk8009C294;

void func_8009C914(Unk8009C294 *this);
void func_8009CB58(Unk8009C294 *this, s32 arg);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C294);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C43C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C4EC);

/* The update of func_8009C294's class: func_8009C914 until 0xB4 is set, then func_8009CB58. */
void func_8009C62C(Unk8009C294 *this, s32 arg) {
    if (this->unkB4 == 0) {
        func_8009C914(this);
    }
    func_8009CB58(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C678);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C898);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009C914);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009CB58);

extern EntityVtable D_8006A184;
extern VtableEntry D_8006A1AC[];
Unk8009BE08 *func_8009BE08(Unk8009BE08 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);

/* Constructs the class of virtual table D_8006A184 (and D_8006A1AC for its base at 0x18) on
 * func_8009BE08's for chara at position, moving by velocity, with 0x6B08000 and 7 in its base's 0x8 and 0xC. */
Unk8009BE08 *func_8009CE8C(Unk8009BE08 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_8009BE08(this, chara, position, velocity);
    this->base.parts.base.vtable = D_8006A1AC;
    this->base.parts.entity.vtable = &D_8006A184;
    func_8007BDC8(&this->base.parts.base, 0x6B08000, 7);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009CEE0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009CFBC);

/* Constructs an entity of the class of virtual table D_8006A15C for arg. */
Unk8009D20C *func_8009D20C(Unk8009D20C *this, void *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A15C;
    this->unk18 = arg;
    this->unk1C = 0;
    this->unk20 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009D25C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009D308);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009D4EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009D5D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009D68C);

/* The destructor of the entity class of vtable D_8006A2AC, which func_80099A7C installs. */
void func_8009D874(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A284, which func_8009AB40 installs. */
void func_8009D8BC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A25C, which func_8009B834 installs. */
void func_8009D904(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_8009D94C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006A1EC and D_8006A1CC, which
 * func_8009C294 installs. */
void func_8009D9EC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006A1CC;
    this->entity.vtable = &D_8006A1EC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_8009DA8C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A15C, which func_8009D20C installs. */
void func_8009DB2C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A134, which func_8009D4EC installs. */
void func_8009DB74(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006A5C4, which derives from func_80076F34's. */
Unk80076F34 *func_8009DBBC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006A5C4;
    return this;
}

/* The destructor of the entity class of vtable D_8006A5C4, which func_8009DBBC installs. */
void func_8009DBF0(Entity *this, s32 flags) {
    this->vtable = &D_8006A5C4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_8009DC44(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009DC6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009DE10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009E050);

void func_8009E364(Unk80076F34 *this);
void func_8009E424(Unk80076F34 *this);
void func_8009E480(Unk80076F34 *this);

/* A virtual function of Unk80076F34: func_8007B9C8; at step 0 sound 0xC3, at step 8 func_8009E424, at step 9
 * func_8009E480 and then func_8009E364. */
void func_8009E244(Unk80076F34 *this, s32 step) {
    func_8007B9C8(this, step);
    if (step == 0) {
        func_800784F4(this, 0xC3);
    }
    if (step == 8) {
        func_8009E424(this);
    }
    if (step == 9) {
        func_8009E480(this);
        func_8009E364(this);
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006A5C4: func_8007BBE8; arg 0 plays
 * sound 0xC3, arg 8 calls func_8009E424, arg 9 func_8009E480 and func_8009E364, and below 10 the position
 * at 0x3B4 moves back by a sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_8009E2B8(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xC3);
    }
    if (arg == 8) {
        func_8009E424(this);
    }
    if (arg == 9) {
        func_8009E480(this);
        func_8009E364(this);
    }
    if (arg < 10) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates an entity of func_8009E8F8's class (0xE4 bytes) at the owner's point 4 moved by D_8006A43C, moving
 * 192 a frame toward func_80079E10's side, hands it to func_8007AD4C and plays sound 0xBE. */
void func_8009E364(Unk80076F34 *this) {
    VECTOR pos;
    VECTOR vel;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(this, &pos, 4, &D_8006A43C);
    vel.vx = func_80079E10(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xE4);
    if (child != NULL) {
        func_8009E8F8(entityOperatorNewAt(0xE4, child), this, obj, &pos, &vel);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xBE);
}

/* Creates an entity of func_8009F248's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8009E424(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F248(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_8009F4D4's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8009E480(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F4D4(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009E4DC);

/* Creates an entity of func_8009F76C's class (0xC0 bytes) 0x180 in front of the character and 0x133
 * up, for func_80079E10's side and arg; hands it to func_8007AD4C and plays sound 0xBF. */
void func_8009E5EC(Unk80076F34 *this, s32 arg) {
    s32 side;
    Vec3 pos;
    Unk8006ED20 *obj;
    Entity *child;

    side = func_80079E10(this);
    func_800775D8(this, &pos);
    pos.vx += side * 0x180;
    pos.vy -= 0x133;
    obj = func_80077580(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_8009F76C(entityOperatorNewAt(0xC0, child), this, obj, &pos, side, arg);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xBF);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009E6C4);

/* Clears 0x4A8 and creates an entity of func_800A0054's class (0xD8 bytes) 0x120 in front of the
 * character, moving 128 a frame toward func_80079E10's side; hands it to func_8007AD4C and plays
 * sound 0xC0. */
void func_8009E744(Unk8009DBBC *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Unk8006ED20 *obj;
    Entity *child;

    this->unk4A8 = 0;
    side = func_80079E10(&this->base);
    func_800775D8(&this->base, &pos);
    vel.vx = side * 128;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x120;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0xD8);
    if (child != NULL) {
        func_800A0054(entityOperatorNewAt(0xD8, child), &this->base, obj, &pos, &vel, 0);
    }
    func_8007AD4C(&this->base, child);
    func_80078540(&this->base, 0xC0);
}

/* As func_8009E744 with 0x4AC, func_800A0830's class (0xD4 bytes), 0x180 in front and 0x180 up,
 * moving 160 a frame. */
void func_8009E814(Unk8009DBBC *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Unk8006ED20 *obj;
    Entity *child;

    this->unk4AC = 0;
    side = func_80079E10(&this->base);
    func_800775D8(&this->base, &pos);
    vel.vx = side * 160;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x180;
    pos.vy -= 0x180;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        func_800A0830(entityOperatorNewAt(0xD4, child), &this->base, obj, &pos, &vel, 0);
    }
    func_8007AD4C(&this->base, child);
    func_80078540(&this->base, 0xC0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009E8F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009E9BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009EB20);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009EB68);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009EF64);

/* Constructs an entity of D_8006A554: func_800C2FBC with entry 9 of D_8006D5C4, 0x19 and 0x802020. */
Entity *func_8009F014(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    func_800C2FBC((Unk800C2F14 *)this, 9, pos, vel, 0x19, 0x802020, 0);
    this->vtable = &D_8006A554;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F064);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F120);

/* Constructs an Unk8009F248 (vtable D_8006A52C) of owner with the sprite frame D_8006A6EC, at the owner's point 4
 * moved by D_8006A44C[1] and moving by D_8006A44C[0]. */
Unk8009F248 *func_8009F248(Unk8009F248 *this, Unk80076F34 *owner) {
    s32 flags = func_8007768C(owner);
    /* The match depends on frame, read into a local before entityInit. */
    u16 *frame = D_8006A6EC;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    func_800C2CCC(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A52C;
    this->owner = owner;
    func_8007A4A4(owner, &pos, 4, &D_8006A44C[1]);
    func_800C3078(&this->base, &pos, &D_8006A44C[0], 0x133, 0x808080);
    this->base.sprite.size = 0x133;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F30C);

/* Entry 0x14 of vtable D_8006A52C: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (func_80079E10), one OT slot further, with buildPolyFT4. */
void func_8009F394(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = func_80079E10(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.pos;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    func_800C3100(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

/* Constructs an Unk8009F248 (vtable D_8006A504) of owner with the sprite frame D_8006A700, at the owner's point 4
 * moved by D_8006A46C[1] and moving by D_8006A46C[0]. */
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Unk80076F34 *owner) {
    s32 flags = func_8007768C(owner);
    /* The match depends on frame, read into a local before entityInit. */
    u16 *frame = D_8006A700;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    func_800C2CCC(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A504;
    this->owner = owner;
    func_8007A4A4(owner, &pos, 4, &D_8006A46C[1]);
    func_800C3078(&this->base, &pos, &D_8006A46C[0], 0x80, 0x808080);
    this->base.sprite.size = 0x33;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F598);

/* Entry 0x14 of vtable D_8006A504: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (func_80079E10), one OT slot further, with buildPolyFT4. */
void func_8009F62C(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = func_80079E10(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.pos;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    func_800C3100(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F76C);

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A4DC, which func_8009F76C installs. */
void func_8009F8E0(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A4DC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009F974);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_8009FC80);

/* The class func_800A0054 constructs (0xD8 bytes, virtual table D_8006A4B4): parts for an owner at pos,
 * moving at vel, with two random words. */
typedef struct Unk800A0054 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *owner;
    /* 0x88 */ Unk8006ED20 *obj; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x8C */ Vec3 pos;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 start;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ Vec3 vel;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0; /* random, 0 to 3 */
    /* 0xC4 */ s32 unkC4; /* random, 0 to 0x7F */
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ s16 vramY; /* the owner's 0x440 (func_8007768C) */
} Unk800A0054;

/* Constructs Unk800A0054 for owner at pos, moving at vel, with arg5, and adds its part to one of obj's two lists
 * (func_80077574 picks which). */
Unk800A0054 *func_800A0054(Unk800A0054 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, Vec3 *vel,
                           s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    this->parts.entity.vtable = &D_8006A4B4;
    func_8007BF20(&this->parts.unk18, owner, 0x6B07000, vel, 3);
    func_80075EE4(&this->parts.part, pos, 0x40, &this->parts.unk18);
    v.vx = func_80079E10(owner) * 1820;
    v.vy = 0;
    v.vz = 0;
    func_8007BDD4(&this->parts.unk18.base, &v);
    this->parts.part.unk14 = &this->parts.entity;
    this->owner = owner;
    this->obj = obj;
    this->parts.part.flags |= 1;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkBC = arg5;
    this->unkC0 = mersenneTwisterGenerate(&obj->random) & 3;
    this->unkC4 = mersenneTwisterGenerate(&obj->random) & 0x7F;
    this->unkC8 = 300;
    this->unkCC = 0;
    this->unkD0 = 0;
    this->vramY = func_8007768C(owner);
    if (func_80077574(owner) == 0) {
        func_8006F044(obj, &this->parts.part.node);
    } else {
        func_8006F024(obj, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A4B4, which func_800A0054 installs. */
void func_800A0214(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A4B4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A02A8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A04BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A0680);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A0774);

/* The class func_800A0830 constructs (0xD4 bytes, virtual table D_8006A48C): parts for an owner at pos,
 * moving at vel, with the owner's facing. */
typedef struct Unk800A0830 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *owner;
    /* 0x88 */ Unk8006ED20 *obj; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x8C */ Vec3 pos;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 start;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ Vec3 vel;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 facing; /* the owner's (func_80079E10) */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s16 vramY; /* the owner's 0x440 (func_8007768C) */
} Unk800A0830;

/* Constructs Unk800A0830 for owner at pos, moving at vel, with arg5 (non-zero also sets bit 0 of the part's
 * flags), and adds its part to one of obj's two lists (func_80077574 picks which). */
Unk800A0830 *func_800A0830(Unk800A0830 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, Vec3 *vel,
                           s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    this->parts.entity.vtable = &D_8006A48C;
    func_8007BF20(&this->parts.unk18, owner, 0x45B07000, (Vec3 *)D_80010814, 3);
    func_80075EE4(&this->parts.part, pos, 0x200, &this->parts.unk18);
    v.vx = func_80079E10(owner) * 0x5550;
    v.vy = -0x5550;
    v.vz = 0;
    func_8007BDD4(&this->parts.unk18.base, &v);
    this->parts.part.unk14 = &this->parts.entity;
    if (arg5 != 0) {
        this->parts.part.flags |= 1;
    }
    this->owner = owner;
    this->obj = obj;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkBC = arg5;
    this->facing = func_80079E10(owner);
    this->vramY = func_8007768C(owner);
    this->unkC4 = 0;
    this->unkC8 = 0x280;
    this->unkCC = 0;
    if (func_80077574(owner) == 0) {
        func_8006F044(obj, &this->parts.part.node);
    } else {
        func_8006F024(obj, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A48C, which func_800A0830 installs. */
void func_800A09F0(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A48C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A0A84);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A0BD0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A0D94);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800A0E58(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0EF8(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0F4C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0FA0(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of vtable D_8006A7FC, which derives from Unk80076F34 and
 * gives it the table D_8006A764. */
Unk800A0FF4 *func_800A0FF4(Unk800A0FF4 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006A7FC;
    func_80079B24(&this->base, D_8006A764);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006A7FC, which func_800A0FF4 installs. */
void func_800A103C(Entity *this, s32 flags) {
    this->vtable = &D_8006A7FC;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800A1090(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1098);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1160);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A11B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1210);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1760);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1818);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A18A4);

/* Spawns the entity of func_800A1E70 64 ahead of and 0x60 above the character, with velocity (96
 * forward, 0, 0), then plays sound 0xC6 and sets 0x334 to 40. */
void func_800A1968(Unk80076F34 *this) {
    s32 facing = func_80079E10(this);
    Unk8006ED20 *unk228;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = facing * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 64;
    position.vy -= 0x60;
    unk228 = func_80077580(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_800A1E70(entityOperatorNewAt(0xC8, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xC6);
    func_80079B0C(this, 40);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1A44);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1CDC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1D84);

/* Creates an entity of the class of vtable D_8006A784 (0x120 bytes) with g++'s new, and hands it
 * to func_8007AD4C. Its constructor func_800A2F64 is inlined. */
void func_800A1DFC(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006A784;
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A1E70);

/* The entity func_800A1E70 constructs (0xC8 bytes, virtual table D_8006A7D4) for the character of
 * func_800A1968 (in the code of func_800A0FF4's class): it sets that character's 0x4A4. */
typedef struct Unk800A1E70 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk800A0FF4 *chara;
} Unk800A1E70;

extern EntityVtable D_8006A7D4;

/* The destructor of the entity class of vtable D_8006A7D4: clears the character's 0x4A4 and calls its
 * func_80079B0C with 1. */
void func_800A1FF4(Unk800A1E70 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006A7D4;
    this->chara->unk4A4 = 0;
    func_80079B0C(&this->chara->base, 1);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.unk18.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A209C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A2244);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A2700);

/* The destructor of the entity class of vtable D_8006A7AC, which func_800A2700 installs. */
void func_800A2798(Entity *this, s32 flags) {
    this->vtable = &D_8006A7AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A27EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A2870);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A2C70);

/* The destructor of the entity class of vtable D_8006A784, which func_800A1DFC and func_800A2F64 install. */
void func_800A2F1C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006A784, which derives from func_800BD490's. */
Entity *func_800A2F64(Entity *this, Unk80076F34 *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006A784;
    return this;
}

/* The character class of virtual table D_8006AA64 (constructor func_800A2F98). */
typedef struct Unk800A2F98 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
} Unk800A2F98;

extern Unk80079B24Entry D_8006A974[];
extern EntityVtable D_8006AA64;

/* Constructs a character of the class of vtable D_8006AA64 (which derives from Unk80076F34) with the
 * table D_8006A974. */
Unk800A2F98 *func_800A2F98(Unk800A2F98 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006AA64;
    this->unk4AC = 0;
    func_80079B24(&this->base, D_8006A974);
    this->unk4A4 = 0;
    this->unk4A8 = 0;
    this->unk4B0 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A2FEC);

/* The override in virtual table D_8006AA64: func_80079B2C with 0. */
void func_800A313C(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A315C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3304);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A34BC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A35C8);

void func_800A3754(Unk80076F34 *this);
void func_800A37F4(Unk80076F34 *this);

/* func_800A3754 when bit 0 of the character's flags at 0x37C is clear, else func_800A37F4. */
void func_800A3710(Unk80076F34 *this) {
    if (func_80077598(this) == 0) {
        func_800A3754(this);
    } else {
        func_800A37F4(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3754);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A37F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A38D8);

/* Creates an entity of func_800A47F4's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800A3A40(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800A47F4(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3A9C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3C98);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3E30);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A3FB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A444C);

/* Spawns the entity of func_800C3654 between 0x84 and 0xA4, as a sibling. */
void func_800A4550(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_800C3654(entityOperatorNewAt(0x5C, child), &this->unk84, &this->unkA4, 0x10, 0x408080, 9);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of func_800C62EC at 0x84, as a sibling. */
void func_800A45C8(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 9, &this->unk84, 0x20, 0x10, 8, 0x408080);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4648);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A47F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4898);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4A08);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4C04);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4D10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4E1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A4F64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5064);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A51DC);

/* The destructor of the entity class of vtable D_8006AA64, which func_800A2F98 installs. */
void func_800A5308(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AA3C and D_8006AA1C, which
 * func_800A3E30 installs. */
void func_800A5350(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AA1C;
    this->entity.vtable = &D_8006AA3C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9F4, which func_800A47F4 installs. */
void func_800A53F0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9CC, which func_800A4C04 installs. */
void func_800A5438(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9A4, which func_800A4F64 installs. */
void func_800A5480(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an Unk800A54C8: func_80076F34's entity with virtual table
 * D_8006B06C, its seven words 0. The game clears 0x4A8-0x4B4 last to first:
 * the match depends on the chained assignment. */
Unk800A54C8 *func_800A54C8(Unk800A54C8 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B06C;
    this->unk4B8 = 0;
    this->unk4A4 = 0;
    this->unk4A8 = this->unk4AC = this->unk4B0 = this->unk4B4 = 0;
    this->unk4BC = 0;
    return this;
}

/* The method at 0x20 of virtual table D_8006B06C: func_80079B0C with 20 while 0x4A4 is set. */
void func_800A5518(Unk800A54C8 *this) {
    if (this->unk4A4 != 0) {
        func_80079B0C(&this->base, 20);
    }
}

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5548(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5588(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 20) when arg is 0. */
void func_800A55C8(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 20);
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5608(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B884: calls it, then func_800A56CC(8) when arg is 15. */
void func_800A5648(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 15) {
        func_800A56CC(this, 8);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 17) when arg is 0. */
void func_800A568C(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 17);
    }
}

/* Creates func_800C5680's entity (0x690 bytes) for the character with D_8006B1AC, 3, 2 and arg, and
 * hands it to func_8007AD4C. */
void func_800A56CC(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_8006B1AC, 3, 2, arg);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A574C);

void func_80079E50(Unk80076F34 *this, VECTOR *arg1, s32 arg2, s32 arg3);
extern VECTOR D_8006AB8C;

/* func_80079E50 with D_8006AB8C, 0x100000 and 0x2AA. */
void func_800A58C8(Unk80076F34 *this) {
    func_80079E50(this, &D_8006AB8C, 0x100000, 0x2AA);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A58F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5AD0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5B98);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5D44);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5EA0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A5F94);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A605C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A60B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6194);

/* The entity class of func_800A6220 (virtual table D_8006B03C), which func_800AA0B4's derives from. */
typedef struct Unk800A6220 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk80075000 *input; /* the character's (func_800776C4) */
    /* 0x20 */ Vec3 position;      /* the character's, at construction */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ s32 unk30; /* 96 the way the character faces */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800A6220;

/* Constructs an entity that follows the character: its input, its position, and 96 the way it faces. */
Unk800A6220 *func_800A6220(Unk800A6220 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B03C;
    this->chara = chara;
    this->input = func_800776C4(chara);
    func_8007A650(chara, &this->position);
    this->unk30 = func_80079E10(chara) * 96;
    this->unk34 = 0;
    this->unk38 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A62A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A63D8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6550);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A65F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6778);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A68FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6AB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6BE4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6CF4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6EB8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A6F8C);

/* The update of the entities of D_8006AF9C: func_800A7290, then func_800A7458. */
void func_800A7254(Entity *this, s32 arg) {
    func_800A7290(this, arg);
    func_800A7458(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7290);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7458);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7694);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7768);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A79D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7C90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7DD4);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg and 0xAC is 0. */
void func_800A7F88(Unk800A7F88 *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL && this->unkAC == 0) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A7FEC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A80B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A8150);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A8274);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A83E0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A84E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A86A8);

/* Constructs an Unk800A8858 for the character, its fields 0. */
Unk800A8858 *func_800A8858(Unk800A8858 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AEDC;
    this->chara = chara;
    this->unk1C[0] = 0;
    this->unk1C[1] = 0;
    this->unk1C[2] = 0;
    this->unk1C[3] = 0;
    this->unk1C[4] = 0;
    this->unk1C[5] = 0;
    this->unk1C[6] = 0;
    this->unk1C[7] = 0;
    this->unk1C[8] = 0;
    this->unk1C[9] = 0;
    this->unk1C[10] = 0;
    this->unk48 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A88D8);

/* The update of the entities of D_8006AEDC: func_800A8C24, then func_800A8F80. */
void func_800A8BE8(Entity *this, s32 arg) {
    func_800A8C24(this, arg);
    func_800A8F80(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A8C24);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A8F80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A926C);

/* The destructor of func_800A926C's class: stops its sound, takes its member at 0x38 out of its list. */
void func_800A9428(Unk800A9944 *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006AE8C;
    this->entity.vtable = &D_8006AEAC;
    stopSound(this->unkD4);
    this->unk38.vtable = D_80065184;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.node;
    this->unk38.node.next->prev = this->unk38.node.prev;
    link->prev->next = this->unk38.node.next;
    this->unk38.node.next = link;
    this->unk38.node.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A94D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A9508);

/* The second base's func_8007BDF0 with arg, then halves 0xB4 when func_80078878 finds an entity in arg. */
void func_800A9944(Unk800A9944 *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unkB4 >>= 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A9998);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A9C4C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A9D0C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800A9D70);

/* The destructor of the entity class of vtable D_8006B06C, which func_800A54C8 installs. */
void func_800A9FBC(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006B03C, which func_800A6220 installs. */
void func_800AA004(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Copies the vector at 0x20 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800AA04C(Unk800AA04C *this, Vec3 *out) {
    s32 x = this->unk20.vx;
    s32 y = this->unk20.vy;
    s32 z = this->unk20.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* The destructor of the entity class of vtable D_8006B00C, which func_800A5AD0 and func_800AA0B4 install. */
void func_800AA06C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006B00C, which derives from func_800A6220's. */
Unk800A6220 *func_800AA0B4(Unk800A6220 *this, Unk80076F34 *chara) {
    func_800A6220(this, chara);
    this->entity.vtable = &D_8006B00C;
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AFE4 and D_8006AFC4, which
 * func_800A65F8 installs. */
void func_800AA0E8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AFC4;
    this->entity.vtable = &D_8006AFE4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF9C, which func_800A6EB8 installs. */
void func_800AA188(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF74, which func_800A7694 installs. */
void func_800AA1D0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AF4C and D_8006AF2C, which
 * func_800A7C90 installs. */
void func_800AA218(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AF2C;
    this->entity.vtable = &D_8006AF4C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF04, which func_800A83E0 installs. */
void func_800AA2B8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AEDC, which func_800A8858 installs. */
void func_800AA300(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AE64, which func_800A9C4C installs. */
void func_800AA348(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of virtual table D_8006B4C4, derived from func_80076F34's. */
Unk800AA390 *func_800AA390(Unk800AA390 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B4C4;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B4C4, which func_800AA390 installs. */
void func_800AA3C8(Entity *this, s32 flags) {
    this->vtable = &D_8006B4C4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_800AA41C(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

void func_80078EF8(Unk80076F34 *this, u32 unk, Vec3 *velocity, s32 unk3);
void func_800C32E0(void *this, Vec3 *position, u32 *unk, s32 unk3, u32 color);
/* offset's initializer in func_800B64E0: GCC keeps it as an anonymous constant (.rodata stays in asm) */
extern VECTOR D_8006C06C;

/* Unk80076F34's method at 0xD4 for virtual table D_8006B4C4: func_8007B978; under arg 24 func_8007835C(0) and the
 * partner (func_8007AEEC) takes the velocities at 0x3A4 and 0x3B4; arg 0 starts the partner's clip 13; arg 24, when
 * the partner is within 0x200 by 0x80, throws it (func_80078EF8) and spawns func_800C32E0's entity at node 9,
 * else stops it (func_80079320). */
void func_800AA444(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Vec3 partnerPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Unk80076F34 *partner;
    Entity *child;
    s32 dx, dy;

    func_8007B978(this, arg);
    partner = func_8007AEEC(this);
    if (arg < 24) {
        s32 x, y, z;

        func_8007835C(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        partner->unk3A4.vx = x;
        partner->unk3A4.vy = y;
        partner->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        partner->unk3B4.vx = x;
        partner->unk3B4.vy = y;
        partner->unk3B4.vz = z;
    }
    switch (arg) {
    case 0:
        func_80079274(partner, 24);
        func_800796F0(partner, 13, 0, 0);
        break;
    case 24:
        func_800775D8(this, &position);
        func_8007A69C(this, &partnerPosition);
        dx = ABS(position.vx - partnerPosition.vx);
        dy = ABS(position.vy - partnerPosition.vy);
        if (dx < 0x200 && dy < 0x80) {
            velocity.vx = (s16)func_80079E10(this) * 18200;
            velocity.vy = -0x4718;
            velocity.vz = 0;
            func_80078EF8(partner, 0xC38C3400, &velocity, 0);
            memset(&offset, 0, sizeof(offset));
            func_8007A4A4(this, (VECTOR *)&node, 9, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                func_800C32E0(entityOperatorNewAt(0x48, child), &node, D_80010814, 0xC0, 0x808080);
            }
            func_8007AD4C(this, child);
        } else {
            func_80079274(partner, 0);
            func_80079320(this);
        }
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AA654);

void func_800AAB4C(Unk80076F34 *this, s32 arg);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006B4C4: func_8007B9C8; arg 0 plays
 * sound 0xDF, arg 7 calls func_800AAB4C with -21 and plays sound 0xDB, and below 8 the x at 0x3B4 moves by
 * a sixteenth of the x at 0x3A4, backwards. */
void func_800AA9F0(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 0) {
        func_800784F4(this, 0xDF);
    }
    if (arg == 7) {
        func_800AAB4C(this, -21);
        func_80078540(this, 0xDB);
    }
    if (arg < 8) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006B4C4: func_8007BBE8 and
 * func_80079AE4; arg 0 plays sounds 0xDF and 0xDB, arg 7 calls func_800AAB4C with 0 and pushes vx at 0x3B4
 * back by 0x3000; below 8 the vector at 0x3B4 slows by the one at 0x3A4 (an eighth of x, all of a positive
 * y). */
void func_800AAA84(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 0) {
        func_800784F4(this, 0xDF);
        func_80078540(this, 0xDB);
    }
    if (arg == 7) {
        func_800AAB4C(this, 0);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 8) {
        this->unk3B4.vx -= this->unk3A4.vx >> 3;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AAB4C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AACC0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AADB0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AAF30);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AB09C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AB4EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AB630);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B49C and D_8006B47C, which
 * func_800AB630 installs. */
void func_800AB790(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B47C;
    this->entity.vtable = &D_8006B49C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AB830);

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AB9F4(UnkPartsEntity38 *this, void *arg) {
    func_8007BDF0(&this->base, arg);
    func_800AB830(this);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ABA24);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ABB54);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ABD08);

/* The class func_800ABE6C constructs (virtual tables D_8006B454, and D_8006B434 for its base at 0x18). */
typedef struct Unk800ABE6C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ Unk80076F34 *chara;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ Vec3 velocity;
    /* 0xB0 */ u8 unkB0[0xB4 - 0xB0];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s16 facing; /* the character's (func_80079E10) */
    /* 0xBA */ s16 vramY;  /* the character's 0x440 (func_8007768C) */
    /* 0xBC */ s16 unkBC;
} Unk800ABE6C;

/* Constructs Unk800ABE6C for chara at position, moving by velocity, and adds the part at 0x38 to one of list's two
 * lists (func_80077574 picks which). */
Unk800ABE6C *func_800ABE6C(Unk800ABE6C *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x7B00100, (Vec3 *)velocity, 0);
    this->unk18.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
    func_80075EE4(&this->unk38, position, 0x40, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
    this->facing = func_80079E10(chara);
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->unkB4 = 0;
    this->unkBC = 0;
    this->vramY = func_8007768C(chara);
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B454 and D_8006B434, which
 * func_800ABE6C installs. */
void func_800ABFE0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AC080);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AC138);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AC21C);

/* Constructs an Unk800AC470 (vtables D_8006B40C and D_8006B3EC): the same as func_800ACA60. */
Unk800AC470 *func_800AC470(Unk800AC470 *this, Unk80076F34 *owner, VECTOR *pos, VECTOR *vel, Unk8006ED20 *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B3EC;
    this->parts.entity.vtable = &D_8006B40C;
    func_80075EE4(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    func_8007BDD4(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->frames = 0;
    this->unkAC = func_8007768C(owner);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(arg4, &this->parts.part.node);
    } else {
        func_8006F024(arg4, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B40C and D_8006B3EC, which
 * func_800AC470 installs. */
void func_800AC5D4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3EC;
    this->entity.vtable = &D_8006B40C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Spawns an entity of func_800AD5BC's class and one of func_800C62EC's at pos, then deletes itself
 * (func_800ACC64 without the sound). */
void func_800AC674(Unk800AC470 *this) {
    Entity *child;
    s32 vx;

    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AC778(Unk800AC470 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800AC674(this);
}

/* The gather of Unk800AC470 (D_8006B40C): the same as func_800ACDA4, with func_800AC674. */
void func_800AC7A8(Unk800AC470 *this, s32 arg) {
    s32 x, y, z;

    x = this->pos.vx;
    y = this->pos.vy;
    z = this->pos.vz;
    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    this->frames++;
    if (this->frames >= 12) {
        func_800AC674(this);
    }
}

/* The shape buildPolyG4 projects (engine/gfx/prim/build_line_f3.h takes it as void *): four corners and a color
 * for each. */
typedef struct PolyG4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 colors[4];
} PolyG4Shape; /* size 0x24 */

/* PolyFT4Shape's uv words as their u and v bytes: game_classes.h types them u16, and the game stores the bytes
 * (RESULT.md). */
#define UV_BYTES(uv) ((u8 *)&(uv))

/* matrixSetTranslationVec (engine/math/matrix_ops.c), which game.bin inlines. */
static inline MATRIX *matrixSetTranslationVecInline(MATRIX *m, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Entry 0x14 of vtable D_8006B40C: the same as func_800ACE1C with a 0x60 by 0x30 picture from the next
 * texture rows and an orange-to-red trail. */
void func_800AC820(Unk800AC470 *this, Camera *camera) {
    VECTOR pos;
    MATRIX m;
    SVECTOR corners[4];
    PolyFT4Shape sprite;
    PolyG4Shape trail;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 vramY;

    pos = *(VECTOR *)&this->pos;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->vel.vy, this->vel.vx));
    matrixSetTranslationVecInline(&m, &pos);
    setModelMatrix(&m);
    corners[0].vx = corners[2].vx = -0x30;
    corners[1].vx = corners[3].vx = 0x30;
    corners[0].vy = corners[1].vy = -0x18;
    corners[2].vy = corners[3].vy = 0x18;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    sprite.flags = 0;
    sprite.corners[0] = &corners[0];
    sprite.corners[1] = &corners[1];
    sprite.corners[2] = &corners[2];
    sprite.corners[3] = &corners[3];
    sprite.color = 0x808080;
    /* The match depends on the u bytes before the v bytes, each pair set in one chain. */
    UV_BYTES(sprite.uv0)[0] = UV_BYTES(sprite.uv2)[0] = 0x20;
    UV_BYTES(sprite.uv1)[0] = UV_BYTES(sprite.uv3)[0] = 0x3F;
    UV_BYTES(sprite.uv0)[1] = UV_BYTES(sprite.uv1)[1] = 0xD0;
    UV_BYTES(sprite.uv2)[1] = UV_BYTES(sprite.uv3)[1] = 0xDF;
    /* The match depends on reading vramY unsigned (lhu, then sra in getTPage): the field is a u16 row. */
    vramY = (u16)this->unkAC;
    sprite.clut = getClut(0x180, vramY + 0xF4);
    sprite.tpage = getTPage(0, 1, 0x180, vramY);
    packet = buildPolyFT4(&sprite, packet, ot, zShift);
    corners[0].vx = -0x18;
    corners[0].vy = 0;
    corners[1].vx = -0x60;
    corners[1].vy = -0x10;
    corners[2].vx = -0x60;
    corners[2].vy = 0x10;
    corners[3].vy = 0x10;
    trail.corners[0] = &corners[0];
    trail.corners[1] = &corners[1];
    trail.corners[2] = &corners[2];
    trail.corners[3] = &corners[3];
    trail.colors[0] = 0x3C80;
    trail.colors[1] = 0x80;
    trail.colors[2] = 0x80;
    trail.colors[3] = 0;
    trail.flags = 9;
    corners[3].vx = ((this->frames & 3) << 4) - 0x1E0;
    PRIM_BUFFER_FREE = buildPolyG4(&trail, packet, ot, zShift);
}

/* Constructs an Unk800ACA60 (vtables D_8006B37C and D_8006B35C) of owner at pos, moving by vel: its parts
 * go in one of arg4's two lists, by func_80077574 of the owner. */
Unk800ACA60 *func_800ACA60(Unk800ACA60 *this, Unk80076F34 *owner, VECTOR *pos, VECTOR *vel, Unk8006ED20 *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B35C;
    this->parts.entity.vtable = &D_8006B37C;
    func_80075EE4(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    func_8007BDD4(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->frames = 0;
    this->unkAC = func_8007768C(owner);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(arg4, &this->parts.part.node);
    } else {
        func_8006F024(arg4, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B37C and D_8006B35C, which
 * func_800ACA60 installs. */
void func_800ACBC4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B35C;
    this->entity.vtable = &D_8006B37C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of func_800C62EC's at
 * pos, then deletes itself. */
void func_800ACC64(Unk800ACA60 *this) {
    Entity *child;
    s32 vx;

    func_80078540(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800ACD74(Unk800ACA60 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800ACC64(this);
}

/* The gather of Unk800ACA60 (D_8006B37C): moves pos by vel, puts the part there, and after 12 frames
 * calls func_800ACC64. */
void func_800ACDA4(Unk800ACA60 *this, s32 arg) {
    s32 x, y, z;

    x = this->pos.vx;
    y = this->pos.vy;
    z = this->pos.vz;
    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    this->frames++;
    if (this->frames >= 12) {
        func_800ACC64(this);
    }
}

/* Entry 0x14 of vtable D_8006B37C: draws the part at pos, turned along its velocity (ratan2): a 0x60 square
 * textured from the owner's VRAM rows (vramY at 0xAC), then a gouraud quad (blue to black) behind it whose tip
 * flickers with frames. */
void func_800ACE1C(Unk800ACA60 *this, Camera *camera) {
    VECTOR pos;
    MATRIX m;
    SVECTOR corners[4];
    PolyFT4Shape sprite;
    PolyG4Shape trail;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 vramY;

    pos = *(VECTOR *)&this->pos;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->vel.vy, this->vel.vx));
    matrixSetTranslationVecInline(&m, &pos);
    setModelMatrix(&m);
    corners[0].vx = corners[2].vx = -0x30;
    corners[1].vx = corners[3].vx = 0x30;
    corners[0].vy = corners[1].vy = -0x30;
    corners[2].vy = corners[3].vy = 0x30;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    sprite.flags = 0;
    sprite.corners[0] = &corners[0];
    sprite.corners[1] = &corners[1];
    sprite.corners[2] = &corners[2];
    sprite.corners[3] = &corners[3];
    sprite.color = 0x808080;
    /* The match depends on the u bytes before the v bytes, each pair set in one chain. */
    UV_BYTES(sprite.uv0)[0] = UV_BYTES(sprite.uv2)[0] = 0;
    UV_BYTES(sprite.uv1)[0] = UV_BYTES(sprite.uv3)[0] = 0x1F;
    UV_BYTES(sprite.uv0)[1] = UV_BYTES(sprite.uv1)[1] = 0xC0;
    UV_BYTES(sprite.uv2)[1] = UV_BYTES(sprite.uv3)[1] = 0xDF;
    /* The match depends on reading vramY unsigned (lhu, then sra in getTPage): the field is a u16 row. */
    vramY = (u16)this->unkAC;
    sprite.clut = getClut(0x180, vramY + 0xF3);
    sprite.tpage = getTPage(0, 1, 0x180, vramY);
    packet = buildPolyFT4(&sprite, packet, ot, zShift);
    corners[0].vx = -0x18;
    corners[0].vy = 0;
    corners[1].vx = -0x60;
    corners[1].vy = -0x10;
    corners[2].vx = -0x60;
    corners[2].vy = 0x10;
    corners[3].vy = 0x10;
    trail.corners[0] = &corners[0];
    trail.corners[1] = &corners[1];
    trail.corners[2] = &corners[2];
    trail.corners[3] = &corners[3];
    trail.colors[0] = 0x803C00;
    trail.colors[1] = 0x800000;
    trail.colors[2] = 0x800000;
    trail.colors[3] = 0;
    trail.flags = 9;
    corners[3].vx = ((this->frames & 3) << 4) - 0x1E0;
    PRIM_BUFFER_FREE = buildPolyG4(&trail, packet, ot, zShift);
}

/* Constructs Unk800AD050 for chara at position, moving by velocity, keeps where the character is, and adds the
 * part at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AD050 *func_800AD050(Unk800AD050 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, chara, 0, (Vec3 *)velocity, 0);
    this->parts.base.vtable = D_8006B3A4;
    this->parts.entity.vtable = &D_8006B3C4;
    func_80075EE4(&this->parts.part, position, 0x40, &this->parts.base);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    func_800775D8(chara, &this->charaPosition);
    this->unkB8 = 0;
    this->unkBC = 10;
    this->unkC0 = 0;
    this->unkC8 = 0;
    this->unkCC = func_8007768C(chara);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B3C4 and D_8006B3A4, which
 * func_800AD050 installs. */
void func_800AD1D0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3A4;
    this->entity.vtable = &D_8006B3C4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of func_800C62EC's at
 * pos, then deletes itself (the same as func_800ACC64, for Unk800AD050). */
void func_800AD270(Unk800AD050 *this) {
    Entity *child;
    s32 vx;

    func_80078540(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->list, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AD380(Unk800AD050 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800AD270(this);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AD3B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AD450);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AD5BC);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B334 and D_8006B314, which
 * func_800AD5BC installs. */
void func_800AD6F8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B314;
    this->entity.vtable = &D_8006B334;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44. */
void func_800AD798(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    this->unk44 |= 1;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AD7CC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AD848);

/* Constructs an entity of the class of virtual table D_8006B754, derived from func_80076F34's. */
Unk800ADA44 *func_800ADA44(Unk800ADA44 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B754;
    this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B754, which func_800ADA44 installs. */
void func_800ADA7C(Entity *this, s32 flags) {
    this->vtable = &D_8006B754;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_800ADAD0(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ADAF8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ADCE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ADDA0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800ADE90);

/* Creates an entity of func_800AED0C's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800ADFD4(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800AED0C(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006B754: func_8007B9C8, sets bit 6
 * of 0x37C; arg 1 calls func_800AE16C and plays sound 0xE7, arg 20 plays sound 0xE3, and below 21 the x
 * at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_800AE030(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800AE16C(this);
        func_800784F4(this, 0xE7);
    }
    if (arg == 20) {
        func_80078540(this, 0xE3);
    }
    if (arg < 21) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006B754: func_8007BBE8 and
 * func_80079AE4; arg 1 calls func_800AE16C and plays sound 0xE7, arg 20 plays sound 0xE3, and below 21 the
 * position at 0x3B4 moves back by a sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_800AE0C4(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800AE16C(this);
        func_800784F4(this, 0xE7);
    }
    if (arg == 20) {
        func_80078540(this, 0xE3);
    }
    if (arg < 21) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AE16C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AE244);

/* Creates an entity of func_800AFD78's class (0x60 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE438(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x60);

    if (child != NULL) {
        func_800AFD78(entityOperatorNewAt(0x60, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B01AC's class (0x40 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE4A8(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B01AC(entityOperatorNewAt(0x40, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B050C's class (0x40 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE518(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B050C(entityOperatorNewAt(0x40, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* The class func_800AE588 constructs (virtual tables D_8006B72C, and D_8006B70C for its base at 0x18): like
 * Unk800AD050, but it keeps its start position at 0x98 and the velocity's length at 0xC0. */
typedef struct Unk800AE588 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ Unk8006ED20 *list; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 start;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ Vec3 vel;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 speed; /* vecGetLength of vel */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s16 unkCC; /* func_8007768C of the owner */
    /* 0xCE */ s16 unkCE;
} Unk800AE588;

/* Constructs Unk800AE588 for chara at position, moving by velocity (its base gets velocity's x and z times 8
 * and y less 0xC00), and adds the part at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AE588 *func_800AE588(Unk800AE588 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list, s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, chara, 0x7AC0800, (Vec3 *)velocity, 1);
    this->parts.base.vtable = D_8006B70C;
    this->parts.entity.vtable = &D_8006B72C;
    func_80075EE4(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 3;
    v.vy -= 0xC00;
    v.vz <<= 3;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = position->vx;
    sy = position->vy;
    sz = position->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkB8 = 0;
    this->unkBC = 0;
    this->speed = vecGetLength(velocity);
    this->unkC4 = 0;
    this->unkC8 = 0;
    this->unkCE = arg5;
    this->unkCC = func_8007768C(chara);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B72C and D_8006B70C, which
 * func_800AE588 installs. */
void func_800AE734(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B70C;
    this->entity.vtable = &D_8006B72C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AE7D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AE890);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AEAF0);

/* Constructs an entity of the class of vtable D_8006B6E4 for an Unk80076F34, with the vector
 * func_800775D8 gives. */
Unk800AED0C *func_800AED0C(Unk800AED0C *this, Unk80076F34 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B6E4;
    this->unk18 = arg;
    func_800775D8(arg, &this->unk1C);
    this->unk2C = -0x180;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B6E4, which func_800AED0C installs. */
void func_800AED88(Entity *this, s32 flags) {
    this->vtable = &D_8006B6E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B6E4: follows the character's position 16 times, then deletes itself. */
void func_800AEDDC(Unk800AED0C *this, s32 arg) {
    if (this->unk38 >= 16) {
        entityDelete(&this->entity);
    } else {
        this->unk38++;
        func_800775D8(this->unk18, &this->unk1C);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AEE48);

/* The entity class of virtual table D_8006B6BC (constructor func_800AF0A0, spawned by func_800AE890). */
typedef struct Unk800AF0A0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 unk2C;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
} Unk800AF0A0;

/* Constructs the entity of virtual table D_8006B6BC with arg1 and copies of the two vectors. The match
 * depends on reading each vector's three words before storing any, as vecCopy does. */
Unk800AF0A0 *func_800AF0A0(Unk800AF0A0 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B6BC;
    this->unk18 = arg1;
    x = vec1->vx;
    y = vec1->vy;
    z = vec1->vz;
    this->unk1C.vx = x;
    this->unk1C.vy = y;
    this->unk1C.vz = z;
    x2 = vec2->vx;
    y2 = vec2->vy;
    z2 = vec2->vz;
    this->unk2C.vx = x2;
    this->unk2C.vy = y2;
    this->unk2C.vz = z2;
    this->unk3C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B6BC, which func_800AF0A0 installs. */
void func_800AF134(Entity *this, s32 flags) {
    this->vtable = &D_8006B6BC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AF188);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AF214);

/* The entity func_800AE16C spawns (virtual tables D_8006B694, and D_8006B674 for its base at 0x18). */
typedef struct Unk800AF364 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ Unk8006ED20 *list;
    /* 0x088 */ Vec3 position;
    /* 0x094 */ u8 unk94[0x98 - 0x94];
    /* 0x098 */ Vec3 velocity;
    /* 0x0A4 */ u8 unkA4[0x138 - 0xA4];
    /* 0x138 */ s32 unk138;
    /* 0x13C */ s32 unk13C;
    /* 0x140 */ s32 facing; /* the character's (func_80079E10) */
    /* 0x144 */ s32 speed; /* the velocity's length */
    /* 0x148 */ s32 unk148;
    /* 0x14C */ s32 unk14C;
    /* 0x150 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
    /* 0x152 */ u8 unk152[0x154 - 0x152];
    /* 0x154 */ s32 unk154;
} Unk800AF364;

/* Constructs func_800AE16C's entity for the character at position, flying at velocity, and adds its member
 * at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AF364 *func_800AF364(Unk800AF364 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x7A83000, (Vec3 *)velocity, 3);
    this->unk18.vtable = D_8006B674;
    this->entity.vtable = &D_8006B694;
    func_80075EE4(&this->unk38, position, 0x100, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->facing = func_80079E10(chara);
    this->unk138 = 0;
    this->unk13C = 0;
    this->speed = vecGetLength(velocity);
    this->unk148 = 0;
    this->unk14C = 0;
    this->vramY = func_8007768C(chara);
    this->unk154 = 0;
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    this->unk38.flags |= 1;
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B694 and D_8006B674, which
 * func_800AF364 installs. */
void func_800AF504(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B674;
    this->entity.vtable = &D_8006B694;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes) at owner's position: NULL when the allocation
 * fails. */
static inline Entity *func_800AF5A4_newFlash(Unk800AF364 *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of func_800C62EC's burst (0x3C bytes, kind 4, eight particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *func_800AF5A4_newBurst(Unk800AF364 *owner, s32 radius) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 4, &owner->position, radius, 0xA0, 8, 0x808080);
    }
    return child;
}

/* The base at 0x18's first method for func_800AE16C's entity: func_8007BDF0 with sum; when sum holds an entity
 * of kind 0x10000 (func_8002BD68), sets 0x154 and bit 0 of the member at 0x38 and adds a flash at the position
 * when func_80078DE0 holds for the character's partner (func_8007AEEC), else a burst of 8 of radius
 * velocity.vx / 4. */
void func_800AF5A4(Unk800AF364 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->unk18, sum);
    if (sum->unk14 != NULL && func_8002BD68(sum->unk14) == 0x10000) {
        this->unk154 = 1;
        this->unk38.flags |= 1;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, func_80078DE0(func_8007AEEC(this->chara))
                                            ? func_800AF5A4_newFlash(this)
                                            : func_800AF5A4_newBurst(this, this->velocity.vx >> 2));
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AF6F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AF970);

/* Constructs an entity of the class of vtable D_8006B64C for an Unk80076F34 and its
 * func_80077580 object. */
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B64C;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk5E = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B64C, which func_800AFD78 installs. */
void func_800AFDE8(Entity *this, s32 flags) {
    this->vtable = &D_8006B64C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AFE3C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800AFF40);

/* Constructs an entity of the class of vtable D_8006B624 for an Unk80076F34 and its
 * func_80077580 object. */
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B624;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B624, which func_800B01AC installs. */
void func_800B021C(Entity *this, s32 flags) {
    this->vtable = &D_8006B624;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B624: moves 0x24 down by 0x120, 24 times, then deletes it. */
void func_800B0270(Unk800B01AC *this, s32 arg) {
    if (this->unk34 >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk34++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B02E0);

/* Constructs an entity of the class of vtable D_8006B5FC for an Unk80076F34 and its
 * func_80077580 object. */
Unk800B050C *func_800B050C(Unk800B050C *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B5FC;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B5FC, which func_800B050C installs. */
void func_800B057C(Entity *this, s32 flags) {
    this->vtable = &D_8006B5FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B5FC: moves 0x24 down by 0x120, 16 times, then deletes it. */
void func_800B05D0(Unk800B01AC *this, s32 arg) {
    if (this->unk34 >= 16) {
        entityDelete(&this->entity);
    } else {
        this->unk34++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B0640);

/* Constructs an entity of the class of virtual table D_8006BCA4, derived from func_80076F34's. */
Unk800B0854 *func_800B0854(Unk800B0854 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006BCA4;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006BCA4, which func_800B0854 installs. */
void func_800B088C(Entity *this, s32 flags) {
    this->vtable = &D_8006BCA4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B08E0);

/* func_800B08E8's local table of four entries (see Unk800685DC). */
typedef struct Unk8006B87C {
    Unk800C5B18Entry entries[4];
} Unk8006B87C;

extern Unk8006B87C D_8006B87C;

/* Overrides func_8007B884: calls it, and at step 8 creates func_800C5680's entity (0x690 bytes) with a copy of
 * D_8006B87C, 4, 5 and 5, and hands it to func_8007AD4C. */
void func_800B08E8(Unk80076F34 *this, s32 arg) {
    Unk8006B87C table;
    Entity *child;

    table = D_8006B87C;
    func_8007B884(this, arg);
    if (arg == 8) {
        child = allocEntityMemory(0x690);
        if (child != NULL) {
            func_800C5680(entityOperatorNewAt(0x690, child), this, table.entries, 4, 5, 5);
        }
        func_8007AD4C(this, child);
    }
}

/* Unk800B0854's override of Unk80076F34's method at 0xB0: func_8007B784, then
 * func_800B19CC when arg is 5. */
void func_800B09B0(Unk800B0854 *this, s32 arg) {
    func_8007B784(&this->base, arg);
    if (arg == 5) {
        func_800B19CC(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B09F4);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_8006BCA4: on 0, func_80079830 and
 * sound 0xEF; on 10, func_800B1518. */
void func_800B0B08(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_800784F4(this, 0xEF);
        break;
    case 10:
        func_800B1518(this, 0);
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B0B78);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B0CBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B0DD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B0F48);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1084);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1220);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B146C);

/* The gather of the Unk8008E9B4 of D_8006BC7C: func_800C3E64, then puts the world position of the
 * owner's point 3 (func_8007A4A4) at 0x28. */
void func_800B14D0(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    func_8007A4A4(this->unk54, &this->base.unk28, 3, &origin);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1518);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1618);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B16F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B17B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1938);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B19CC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1AC0);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006BC54 and D_8006BC34, which
 * func_800B1AC0 installs. */
void func_800B1C84(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006BC34;
    this->entity.vtable = &D_8006BC54;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B1D24);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg. */
void func_800B1FD4(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2028);

/* The entity class of vtable D_8006BC0C (0x3C bytes), made for a character at a position. */
typedef struct Unk800B2240 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 unk26; /* the character's 0x440 (func_8007768C) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} Unk800B2240;

/* Constructs the entity of vtable D_8006BC0C for a character at a position, with arg3 at 0x1C. */
Unk800B2240 *func_800B2240(Unk800B2240 *this, Unk80076F34 *chara, Vec3 *position, s32 arg3) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BC0C;
    this->unk26 = func_8007768C(chara);
    this->facing = func_80079E10(chara);
    this->chara = chara;
    this->unk24 = 0;
    this->unk22 = 0x80;
    this->unk1E = 0;
    this->unk28 = 0x1000;
    this->unk1C = arg3;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BC0C, which func_800B2240 installs. */
void func_800B22EC(Entity *this, s32 flags) {
    this->vtable = &D_8006BC0C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2340);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2414);

/* The entity class of vtable D_8006BBE4 (0x3C bytes), made for a character at a position. */
typedef struct Unk800B25CC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ s16 unk24; /* the character's 0x440 (func_8007768C) */
    /* 0x26 */ u8 unk26[2];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} Unk800B25CC;

/* Constructs the entity of vtable D_8006BBE4 for a character at a position. */
Unk800B25CC *func_800B25CC(Unk800B25CC *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BBE4;
    this->unk24 = func_8007768C(chara);
    this->facing = func_80079E10(chara);
    this->chara = chara;
    this->unk22 = 0;
    this->unk20 = 0x80;
    this->unk1C = 0;
    this->unk28 = 0;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BBE4, which func_800B25CC installs. */
void func_800B2664(Entity *this, s32 flags) {
    this->vtable = &D_8006BBE4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B26B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B275C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B29C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2A7C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2B40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2BA8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2D10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B2E68);

extern VtableEntry D_8006BB54[];
extern EntityVtable D_8006BB74;

/* The destructor of Unk800B2E68 (virtual tables D_8006BB74 and D_8006BB54): while 0x8A is set,
 * func_80079274 on func_8007AEEC's character first. */
void func_800B304C(Unk800B2E68 *this, s32 flags) {
    ListNode *node;

    this->unk18.vtable = D_8006BB54;
    this->entity.vtable = &D_8006BB74;
    if (this->unk8A != 0) {
        func_80079274(func_8007AEEC(this->chara), 0);
    }
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B310C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B33DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B34D0);

/* The direction (ratan2) from the centre at 0xAC to point index, on x and y. */
s16 func_800B36B4(Unk800B2E68 *this, s32 index) {
    return ratan2(this->unkCC[index].vx - this->unkAC.vx, this->unkCC[index].vy - this->unkAC.vy);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B36F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B3990);

/* The destructor of the entity class of vtable D_8006BB2C, which func_800B3990 installs. */
void func_800B3AC0(Entity *this, s32 flags) {
    this->vtable = &D_8006BB2C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B3B14);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B3CCC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B3F38);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006BB04 and D_8006BAE4, which
 * func_800B3F38 installs. */
void func_800B408C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006BAE4;
    this->entity.vtable = &D_8006BB04;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B412C);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg. */
void func_800B427C(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B42D0);

/* The destructor of the entity class of vtable D_8006BC7C, which func_800B146C installs. */
void func_800B44D8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800B4520(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of vtable D_8006BEE4, which derives from Unk80076F34 and
 * gives it the table D_8006BE1C. */
Unk800B45C0 *func_800B45C0(Unk800B45C0 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006BEE4;
    func_80079B24(&this->base, D_8006BE1C);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006BEE4, which func_800B45C0 installs. */
void func_800B4608(Entity *this, s32 flags) {
    this->vtable = &D_8006BEE4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B465C);

/* Overrides func_8007B884: calls it, then func_8007A6FC(1, 80, 8) when arg is 0. */
void func_800B4664(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 0) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B46AC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B4898);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B491C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B4E28);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B4F10);

/* Creates func_800B50EC's entity (0x160 bytes) 0xB0 above the character, moving 48 a frame toward
 * func_80079E10's side; hands it to func_8007AD4C and calls func_80079B0C(40). */
void func_800B4FA4(Unk80076F34 *this) {
    Unk8006ED20 *obj;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    obj = func_80077580(this);
    func_800775D8(this, &position);
    position.vy -= 0xB0;
    velocity.vx = func_80079E10(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        func_800B50EC(entityOperatorNewAt(0x160, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
    func_80079B0C(this, 40);
}

/* Creates func_800B5920's entity (0x74 bytes) at the character's point 9 moved by D_800D345C, and
 * hands it to func_8007AD4C. */
void func_800B5060(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR position;
    Entity *child;

    func_8007A4A4(this, &position, 9, &D_800D345C);
    child = allocEntityMemory(0x74);
    if (child != NULL) {
        func_800B5920(entityOperatorNewAt(0x74, child), this, obj, &position);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B50EC);

/* The destructor of func_800B50EC's class: stops its sound if it plays, takes its member at 0x38 out of
 * its list. */
void func_800B5320(Unk800B50EC *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006BE9C;
    this->entity.vtable = &D_8006BEBC;
    if (this->unk15C != 0) {
        stopSound(this->unk15C);
    }
    this->unk38.vtable = D_80065184;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.node;
    this->unk38.node.next->prev = this->unk38.node.prev;
    link->prev->next = this->unk38.node.next;
    this->unk38.node.next = link;
    this->unk38.node.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B53D0);

/* func_80078DE0 of a target, after taking its position (func_800775D8), which is not used. */
static inline s32 func_800B55EC_targetTest(Unk80076F34 *target) {
    Vec3 position;

    func_800775D8(target, &position);
    return func_80078DE0(target);
}

/* The base at 0x18's first method for func_800B50EC's class: func_8007BDF0 with sum; when func_80078878 finds a
 * target in sum for which func_80078DE0 holds, func_800C75FC's entity on it, else a burst of 6 (func_800C62EC) at
 * 0xC8; then the entity deletes itself. */
void func_800B55EC(Unk800B50EC *this, Unk80075EE4 *sum) {
    Entity *target;
    Entity *child;

    func_8007BDF0(&this->unk18, sum);
    target = func_80078878(sum);
    /* The match depends on the target test as an inline beside the null test (an if/else, one delete). */
    if (target != NULL && func_800B55EC_targetTest((Unk80076F34 *)target)) {
        entityAddSibling(&this->entity, func_800C75FC(target, &this->velocity, 0x60));
    } else {
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unkC8, this->speed >> 1, 0x60, 6, 0x808080);
        }
        entityAddSibling(&this->entity, child);
    }
    entityDelete(&this->entity);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B56EC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B5920);

/* The destructor of the entity class of vtable D_8006BE74, which func_800B5920 installs. */
void func_800B5A1C(Entity *this, s32 flags) {
    this->vtable = &D_8006BE74;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B5A70);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B5C90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B5F14);

/* The destructor of the entity class of vtable D_8006BE4C, which func_800B5F14 installs. */
void func_800B5FA8(Entity *this, s32 flags) {
    this->vtable = &D_8006BE4C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B5FFC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B6130);

/* Constructs an entity of the class of vtable D_8006C15C, which derives from Unk80076F34 and
 * gives it the table D_8006C05C. */
Unk800B631C *func_800B631C(Unk800B631C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006C15C;
    func_80079B24(&this->base, D_8006C05C);
    this->unk4A4 = this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006C15C, which func_800B631C installs. */
void func_800B6368(Entity *this, s32 flags) {
    this->vtable = &D_8006C15C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B63BC);

/* Unk80076F34's method at 0xD4 for virtual table D_8006C15C: func_8007B978; under arg 10 func_8007835C(0) and the
 * partner (func_8007AEEC) takes the velocities at 0x3A4 and 0x3B4; arg 0 starts the partner's clip 13; arg 10, when
 * the partner is within 0x180 by 0x80, throws it (func_80078EF8) and spawns func_800C32E0's entity at node 12,
 * else stops it (func_80079320). */
void func_800B64E0(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Vec3 partnerPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Unk80076F34 *partner;
    Entity *child;
    s32 dx, dy;

    func_8007B978(this, arg);
    partner = func_8007AEEC(this);
    if (arg < 10) {
        s32 x, y, z;

        func_8007835C(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        partner->unk3A4.vx = x;
        partner->unk3A4.vy = y;
        partner->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        partner->unk3B4.vx = x;
        partner->unk3B4.vy = y;
        partner->unk3B4.vz = z;
    }
    switch (arg) {
    case 0:
        func_80079274(partner, 10);
        func_800796F0(partner, 13, 0, 0);
        break;
    case 10:
        func_800775D8(this, &position);
        func_8007A69C(this, &partnerPosition);
        dx = ABS(position.vx - partnerPosition.vx);
        dy = ABS(position.vy - partnerPosition.vy);
        if (dx < 0x180 && dy < 0x80) {
            velocity.vx = (s16)func_80079E10(this) * 14560;
            velocity.vy = -0x2AA8;
            velocity.vz = 0;
            func_80078EF8(partner, 0xC38C1B58, &velocity, 0);
            offset = D_8006C06C;
            func_8007A4A4(this, (VECTOR *)&node, 12, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                func_800C32E0(entityOperatorNewAt(0x48, child), &node, D_80010814, 0xC0, 0x808080);
            }
            func_8007AD4C(this, child);
        } else {
            func_80079274(partner, 0);
            func_80079320(this);
        }
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006C15C: func_8007B9C8, sets bit 6
 * of 0x37C; arg 1 calls func_800B6910 and plays sound 0xFB, arg 13 calls func_800B6868 and plays sound
 * 0xF7, and below 14 the x at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_800B66FC(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800B6910(this);
        func_800784F4(this, 0xFB);
    }
    if (arg == 13) {
        func_800B6868(this);
        func_80078540(this, 0xF7);
    }
    if (arg < 14) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006C15C: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 1 calls func_800B6910 and plays sound 0xFB; arg 13 calls func_800B6868, plays sound 0xF7 and pushes vx at 0x3B4 back by
 * 0x3000; below 14 the vector at 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_800B6798(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800B6910(this);
        func_800784F4(this, 0xFB);
    }
    if (arg == 13) {
        func_800B6868(this);
        func_80078540(this, 0xF7);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 14) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_800B719C's entity (0xC0 bytes) 0x80 above the character, moving 128 a frame toward
 * func_80079E10's side, and hands it to func_8007AD4C. */
void func_800B6868(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x80;
    velocity.vx = func_80079E10(this) * 128;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_800B719C(entityOperatorNewAt(0xC0, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B73E8's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800B6910(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800B73E8(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

/* A step function of the class of func_800B631C: at step 0, sounds 0xFC and 0xF8 and clears unk4A4 and unk4A8;
 * counts unk4A4 down to 0; on odd steps before 16, func_800B6D7C; on steps 4 to 29 while unk4A4 is 0, creates a
 * func_800B6DD8 entity (0x94 bytes, color by unk4A8) at point 5 (odd steps) or 12 of the body; before step 31,
 * func_8007835C toward func_80079E10's side and unk3A4.vy = -step * 256, else func_8007835C with 0; before step 3,
 * func_8007A6FC. */
void func_800B696C(Unk800B631C *this, s32 step) {
    Unk8006ED20 *unk228;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 color;
    s32 odd;

    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        func_800784F4(&this->base, 0xFC);
        func_80078540(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = step & 1;
    if (odd && step < 16) {
        func_800B6D7C(&this->base);
    }
    if (step < 30 && this->unk4A4 == 0 && step >= 4) {
        unk228 = func_80077580(&this->base);
        memset(&offset, 0, sizeof(offset));
        color = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            color = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before color. */
        if (odd) {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        } else {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        }
    }
    if (step < 31) {
        func_8007835C(&this->base, func_80079E10(&this->base) << 14);
        this->base.unk3A4.vy = -(step << 8);
    } else {
        func_8007835C(&this->base, 0);
    }
    if (step < 3) {
        func_8007A6FC(&this->base, -1, 0x60, 5);
    }
}

/* A step function of the class of func_800B631C: at step 0, sounds 0xFC and 0xF8 and clears unk4A4 and unk4A8;
 * counts unk4A4 down to 0; on odd steps before 16, func_800B6D7C; on steps 0 to 29 while unk4A4 is 0, creates a
 * func_800B6DD8 entity (0x94 bytes, color by unk4A8) at point 5 (odd steps) or 12 of the body; before step 31,
 * func_8007835C toward func_80079E10's side and unk3A4.vy = -step * 256, else func_8007835C with 0. */
void func_800B6B88(Unk800B631C *this, s32 step) {
    Unk8006ED20 *unk228;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 color;
    s32 odd;

    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        func_800784F4(&this->base, 0xFC);
        func_80078540(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = step & 1;
    if (odd && step < 16) {
        func_800B6D7C(&this->base);
    }
    if (step < 30 && this->unk4A4 == 0) {
        unk228 = func_80077580(&this->base);
        memset(&offset, 0, sizeof(offset));
        color = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            color = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before color. */
        if (odd) {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        } else {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        }
    }
    if (step < 31) {
        func_8007835C(&this->base, func_80079E10(&this->base) << 14);
        this->base.unk3A4.vy = -(step << 8);
    } else {
        func_8007835C(&this->base, 0);
    }
}

/* Creates an entity of func_800B7820's class (0x120 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800B6D7C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        func_800B7820(entityOperatorNewAt(0x120, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B6DD8);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006C134 and D_8006C114, which
 * func_800B6DD8 installs. */
void func_800B6F24(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C114;
    this->entity.vtable = &D_8006C134;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B6FC4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7158);

/* Constructs func_800C7024's entity with 0x80, 0x7A81800, 0x2000, 3, then the virtual tables
 * D_8006C0EC (and D_8006C0CC for the base at 0x18). */
Unk800C7024 *func_800B719C(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity,
                          Unk8006ED20 *obj) {
    func_800C7024(this, chara, position, velocity, 0x80, 0x7A81800, 0x2000, 3);
    this->parts.base.vtable = D_8006C0CC;
    this->parts.entity.vtable = &D_8006C0EC;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7200);

/* Constructs an entity of the class of vtable D_8006C0A4 for an Unk80076F34, with the vector
 * func_800775D8 gives. */
Unk800B73E8 *func_800B73E8(Unk800B73E8 *this, Unk80076F34 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C0A4;
    this->unk18 = arg;
    func_800775D8(arg, &this->unk1C);
    this->unk2C = 0x40;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006C0A4, which func_800B73E8 installs. */
void func_800B7464(Entity *this, s32 flags) {
    this->vtable = &D_8006C0A4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006C0A4: follows the character's position, 0x60 above it, 13 times, then
 * deletes itself. */
void func_800B74B8(Unk800B73E8 *this, s32 arg) {
    if (this->unk38 >= 13) {
        entityDelete(&this->entity);
    } else {
        this->unk38++;
        func_800775D8(this->unk18, &this->unk1C);
        this->unk1C.vy -= 0x60;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7538);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7820);

/* The destructor of the entity class of vtable D_8006C07C, which func_800B7820 installs. */
void func_800B79D4(Entity *this, s32 flags) {
    this->vtable = &D_8006C07C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7A28);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7BB0);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800B7E3C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of vtable D_8006C384, which derives from Unk80076F34 and
 * gives it the table D_8006C324. */
Unk80076F34 *func_800B7EDC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006C384;
    func_80079B24(this, D_8006C324);
    return this;
}

/* The destructor of the entity class of vtable D_8006C384, which func_800B7EDC installs. */
void func_800B7F20(Entity *this, s32 flags) {
    this->vtable = &D_8006C384;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B7F74);

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in D_8006C384: calls it; when arg is 3, moves unk3B4 by unk3C4 * 0x2AA8 (16.16)
 * toward func_80079E10's side; from 4 on, moves unk3B4 back by a quarter of unk3A4. */
void func_800B7F80(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 3) {
        s32 step;

        step = ((s64)this->unk3C4 * 0x2AA8) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 4) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

/* Overrides func_8007B70C (entry 0xAC of Unk80076F34's vtable) in D_8006C384: calls it; when arg is 4,
 * moves unk3B4 by unk3C4 * 0x38E0 (16.16) toward func_80079E10's side; from 5 on, moves unk3B4 back
 * by a quarter of unk3A4. */
void func_800B8030(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 4) {
        s32 step;

        step = ((s64)this->unk3C4 * 0x38E0) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 5) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B80E0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B82B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B8480);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B8670);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B8814);

/* Creates an entity of func_800B8BBC's class (0x58 bytes) at the owner's point 2 moved by D_800D348C,
 * with arg in the vector's pad, and hands it to func_8007AD4C. */
void func_800B8AA8(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR pos;
    Entity *child;

    func_8007A4A4(this, &pos, 2, &D_800D348C);
    pos.pad = arg;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_800B8BBC(entityOperatorNewAt(0x58, child), this, &pos, obj);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800B8FAC's entity (0x60 bytes) at the character's position, and hands it to
 * func_8007AD4C. */
void func_800B8B3C(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    child = allocEntityMemory(0x60);
    if (child != NULL) {
        func_800B8FAC(entityOperatorNewAt(0x60, child), this, &position, obj);
    }
    func_8007AD4C(this, child);
}

/* The entity func_800B8AA8 makes (vtable D_8006C35C, 0x58 bytes): a square of side 0x100 facing the
 * character's way, with the pad of its position vector. */
typedef struct Unk800B8BBC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *obj;
    /* 0x20 */ s16 unk20; /* the pad of the position (func_800B8AA8's arg) */
    /* 0x22 */ s16 unk22; /* func_8007768C of the character */
    /* 0x24 */ s16 unk24; /* minus the character's facing */
    /* 0x26 */ s16 unk26;
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ u8 unk48[0x58 - 0x48];
} Unk800B8BBC;

/* Constructs func_800B8AA8's entity for chara: a square from (-0x80, -0x80) to (0x80, 0x80) at z -0x80. */
Unk800B8BBC *func_800B8BBC(Unk800B8BBC *this, Unk80076F34 *chara, VECTOR *pos, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C35C;
    this->chara = chara;
    this->obj = obj;
    this->unk26 = 0;
    this->unk22 = func_8007768C(this->chara);
    this->unk24 = -func_80079E10(this->chara);
    this->unk20 = pos->pad;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0x80;
    this->corners[1].vx = this->corners[3].vx = 0x80;
    this->corners[0].vy = this->corners[1].vy = -0x80;
    this->corners[2].vy = this->corners[3].vy = 0x80;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x80;
    return this;
}

/* The destructor of the entity class of vtable D_8006C35C, which func_800B8BBC installs. */
void func_800B8C80(Entity *this, s32 flags) {
    this->vtable = &D_8006C35C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B8CD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B8D5C);

/* The entity func_800B8B3C makes (vtable D_8006C334, 0x60 bytes): a flat square of side 0x140 facing the
 * character's way, at a position. */
typedef struct Unk800B8FAC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *obj;
    /* 0x20 */ s16 unk20; /* func_8007768C of the character */
    /* 0x22 */ s16 unk22; /* minus the character's facing */
    /* 0x24 */ s16 unk24;
    /* 0x26 */ u8 unk26[0x28 - 0x26];
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ u8 unk48;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ s16 unk4C;
    /* 0x50 */ Vec3 position;
} Unk800B8FAC;

/* Constructs func_800B8B3C's entity for chara: a square from (-0xA0, -0xA0) to (0xA0, 0xA0) at z 0, at
 * position raised by 0x80. */
Unk800B8FAC *func_800B8FAC(Unk800B8FAC *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C334;
    this->chara = chara;
    this->obj = obj;
    this->unk24 = 0;
    this->unk20 = func_8007768C(this->chara);
    this->unk22 = -func_80079E10(this->chara);
    x = position->vx;
    y = position->vy - 0x80;
    z = position->vz;
    this->unk48 = 0;
    this->unk4A = 0;
    this->unk4C = 0;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0xA0;
    this->corners[1].vx = this->corners[3].vx = 0xA0;
    this->corners[0].vy = this->corners[1].vy = -0xA0;
    this->corners[2].vy = this->corners[3].vy = 0xA0;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The destructor of the entity class of vtable D_8006C334, which func_800B8FAC installs. */
void func_800B9090(Entity *this, s32 flags) {
    this->vtable = &D_8006C334;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B90E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B926C);

/* Constructs an entity of the class of vtable D_8006C71C, which derives from Unk80076F34 and
 * gives it the table D_8006C4FC. */
Unk800B9410 *func_800B9410(Unk800B9410 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006C71C;
    func_80079B24(&this->base, D_8006C4FC);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006C71C, which func_800B9410 installs. */
void func_800B9458(Entity *this, s32 flags) {
    this->vtable = &D_8006C71C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800B9410's override of the method at 0x78: func_8007B2C0, then, the first
 * time, spawns the entity of func_800BAAA4 (with 0) for the character. */
void func_800B94AC(Unk800B9410 *this, s32 arg) {
    func_8007B2C0(&this->base, arg);
    if (this->unk4A4 == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            func_800BAAA4(entityOperatorNewAt(0x4C, child), &this->base, 0);
        }
        func_8007AD4C(&this->base, child);
        this->unk4A4 = 1;
    }
}

/* Unk800B9410's override of the method at 0x80: func_8007B2EC, then, the first
 * time, spawns the entity of func_800BAAA4 (with 1) for the character. */
void func_800B9528(Unk800B9410 *this, s32 arg) {
    func_8007B2EC(&this->base, arg);
    if (this->unk4A4 == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            func_800BAAA4(entityOperatorNewAt(0x4C, child), &this->base, 1);
        }
        func_8007AD4C(&this->base, child);
        this->unk4A4 = 1;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B95A4);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_8006C71C: on 0, func_80079830 and
 * sound 0x105; on 13, func_800B9F24. */
void func_800B972C(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_800784F4(this, 0x105);
        break;
    case 13:
        func_800B9F24(this, 0);
        break;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B979C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B99A8);

/* Overrides func_8007B80C (entry 0xBC of Unk80076F34's vtable) in D_8006C71C: calls it; when arg is 4,
 * clears unk3A4 and moves unk3B4 by unk3C4 * 0x38E0 (16.16) toward func_80079E10's side; from 5 on,
 * moves unk3B4 back by a quarter of unk3A4. */
void func_800B9A64(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 4) {
        s32 step;

        this->unk3A4.vx = 0;
        step = ((s64)this->unk3C4 * 0x38E0) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 5) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B9B18);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B9C48);

/* func_800B9E58's local table of three entries (see Unk800685DC). */
typedef struct Unk8006C5FC {
    Unk800C5B18Entry entries[3];
} Unk8006C5FC;

extern Unk8006C5FC D_8006C5FC;

/* Creates func_800C5680's entity (0x690 bytes) for the character with a copy of D_8006C5FC, 3, 8 and 23, hands
 * it to func_8007AD4C, and plays sound 0x102. */
void func_800B9E58(Unk80076F34 *this) {
    Unk8006C5FC table;
    Entity *child;

    table = D_8006C5FC;
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, table.entries, 3, 8, 23);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x102);
}

/* Spawns the entity of func_800B9FF0 0x70 above the character (0x60 when arg is 0), with velocity
 * (48 forward, 0, 0). */
void func_800B9F24(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    if (arg == 0) {
        position.vy -= 0x60;
    } else {
        position.vy -= 0x70;
    }
    velocity.vx = func_80079E10(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x184);
    if (child != NULL) {
        func_800B9FF0(entityOperatorNewAt(0x184, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800B9FF0);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006C6CC and D_8006C6AC, which
 * func_800B9FF0 installs. */
void func_800BA1F8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C6AC;
    this->entity.vtable = &D_8006C6CC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA298);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA49C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA598);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA734);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA7AC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BA8AC);

/* The entity class func_800BAAA4 constructs (0x4C bytes, virtual table D_8006C684) for an Unk800B9410,
 * which makes one while its 0x4A4 is 0. */
typedef struct Unk800BAAA4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800B9410 *chara;
    /* 0x1C */ u32 handle; /* the character's (entityGetHandle) */
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 facing; /* func_80079E10 */
    /* 0x24 */ s16 vramY;  /* func_8007768C */
    /* 0x26 */ s16 unk26;
    /* 0x28 */ u8 unk28[0x4C - 0x28];
} Unk800BAAA4;

extern EntityVtable D_8006C684;

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BAAA4);

/* The destructor of func_800BAAA4's class: clears the character's 0x4A4, so that it can make another. */
void func_800BAC08(Unk800BAAA4 *this, s32 flags) {
    this->entity.vtable = &D_8006C684;
    this->chara->unk4A4 = 0;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BAC64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BAE98);

/* The destructor of the entity class of vtable D_8006C6F4, which func_800BA734 installs. */
void func_800BAFEC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an Unk800BB034: func_80076F34's entity with virtual table
 * D_8006CB44, its eight words 0 but 0x4B0, which is 1. */
Unk800BB034 *func_800BB034(Unk800BB034 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006CB44;
    this->unk4A4 = 0;
    this->unk4A8 = 0;
    this->unk4AC = 0;
    this->unk4B0 = 1;
    this->unk4B4 = 0;
    this->unk4B8 = 0;
    this->unk4BC = 0;
    this->unk4C0 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB08C);

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB0AC(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 16, 2, 8, 0x404040);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 4. */
void func_800BB104(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 4) {
        func_800BB2B8(this, 5, 2, 6, 0x404040);
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 0. */
void func_800BB15C(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800BB2B8(this, 5, 2, 8, 0x404040);
    }
}

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB1B0(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 5, 3, 14, 0xFFFFFF);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 3. */
void func_800BB208(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 3) {
        func_800BB2B8(this, 5, 2, 8, 0x404040);
    }
}

/* Overrides func_8007BB64 (entry 0x108 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB260(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 5, 3, 14, 0xFFFFFF);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB2B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB3CC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB5F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB78C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB850);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BB9A4);

void func_800BC0CC(Unk800BB034 *this, s32 arg);
void func_800BC0E0(Unk800BB034 *this, s32 arg);
void func_800BC13C(Unk800BB034 *this);
void func_800BC258(Unk800BB034 *this);
void func_800BC424(Unk800BB034 *this, s32 angle);
s32 func_800BC444(Unk800BB034 *this, s32 *angle, s32 from);

/* The method at 0xE4 of Unk800BB034 (virtual table D_8006CB44): on the first call (arg 0) plays sound 0x10E
 * or 0x120, starts with func_800BC0CC(2), func_800BC0E0(0x18) and the angle func_800BC444 gives (kept when
 * within 0x400 of straight ahead); then by its frame count at 0x4C0 turns toward 0x3E0 (frame 8),
 * finishes (frame 9), mirrors 0x3A4 into 0x3B4 (frames 0-2), looks again from frame 11. */
void func_800BBD18(Unk800BB034 *this, s32 arg) {
    s32 angle;
    s32 target;
    s32 facing;

    func_80079AE4(&this->base);
    if (arg == 0) {
        if (func_80077568(&this->base) == 0x15) {
            func_800784F4(&this->base, 0x10E);
        } else {
            func_800784F4(&this->base, 0x120);
        }
        this->unk4B0 = 1;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x18);
        facing = func_80079E10(&this->base);
        /* The match depends on shifting the facing at each use: the shift then comes after the
         * other arguments */
        func_800BC444(this, &angle, facing << 10);
        this->unk4B8 = abs(angle) <= 0x400 ? angle : facing << 10;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.unk3E0 + ((((this->unk4B8 - this->base.unk3E0 - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 9) {
        this->base.unk3B4.vx = -this->base.unk3A4.vx;
        this->base.unk3B4.vy = -this->base.unk3A4.vy;
        this->base.unk3B4.vz = -this->base.unk3A4.vz;
        this->base.unk3C0 = 0;
    }
    if (this->unk4C0 >= 0x11 && func_800BC444(this, &target, this->base.unk3E0)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 10) {
        func_800BC258(this);
    }
    if (func_80079A0C(&this->base)) {
        this->base.unk3F0 = 0;
    }
    this->unk4C0++;
}

/* The method at 0x11C of Unk800BB034: func_800BBD18 with func_800BC0E0(0x19) and 0x4B0 cleared first,
 * keeping func_800BC444's angle as it is, and frames 3 and 11 for 9 and 0x11. */
void func_800BBF00(Unk800BB034 *this, s32 arg) {
    s32 angle;
    s32 target;

    func_80079AE4(&this->base);
    if (arg == 0) {
        if (func_80077568(&this->base) == 0x15) {
            func_800784F4(&this->base, 0x10E);
        } else {
            func_800784F4(&this->base, 0x120);
        }
        this->unk4B0 = 0;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x19);
        angle = func_80079E10(&this->base) << 10;
        func_800BC444(this, &angle, angle);
        this->unk4B8 = angle;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.unk3E0 + ((((this->unk4B8 - this->base.unk3E0 - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 3) {
        this->base.unk3B4.vx = -this->base.unk3A4.vx;
        this->base.unk3B4.vy = -this->base.unk3A4.vy;
        this->base.unk3B4.vz = -this->base.unk3A4.vz;
        this->base.unk3C0 = 0;
    }
    if (this->unk4C0 >= 11 && func_800BC444(this, &target, this->base.unk3E0)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 4) {
        func_800BC258(this);
    }
    if (func_80079A0C(&this->base)) {
        this->base.unk3F0 = 0;
    }
    this->unk4C0++;
}

/* Sets 0x4AC, 0x4B4 to arg, and clears 0x448 of the character. */
void func_800BC0CC(Unk800BB034 *this, s32 arg) {
    this->unk4AC = 1;
    this->unk4B4 = arg;
    this->base.unk448 = 0;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC0E0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC13C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC258);

/* Creates an entity of func_800BD490's class (0x120 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800BC3C8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        func_800BD490(entityOperatorNewAt(0x120, child), this);
    }
    func_8007AD4C(this, child);
}

/* While 0x4B4 is positive, sets the character's angle (0x3E0) and 0x3F0 to angle. */
void func_800BC424(Unk800BB034 *this, s32 angle) {
    if (this->unk4B4 > 0) {
        this->base.unk3E0 = angle;
        this->base.unk3F0 = angle;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC444);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC59C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC6F4);

/* Creates an entity of func_800BDA50's class (0xD4 bytes) at pos with g++'s new and hands it to func_8007AD4C, then
 * plays sound 0x109, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800BC7CC(Unk80076F34 *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800BDA50(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x300);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x109);
    this->unk498 = 10;
    func_80077448(this, -0x10000);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC870);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BC9DC);

/* The second base's first method for virtual table D_8006CAD4: func_8007BDF0 with sum, then deletes the
 * entity when func_80078878 finds an entity in sum. */
void func_800BCA70(UnkPartsEntity38 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->base, sum);
    if (func_80078878(sum) != NULL) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCAD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCB4C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCBD8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCC7C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCE1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCE94);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BCFBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD07C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD1C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD254);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD334);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD490);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD650);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BD8DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BDA50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BDB80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BDF28);

/* The method at 0x20 of virtual table D_8006C9FC: adds func_800BE39C's and func_800C6088's entities at 0xA4
 * as siblings, then plays sound 0x10A. */
void func_800BDFCC(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB08000, 0x700, &D_8006C964);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C6088(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0x700, 0x100, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x10A, 0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BE0C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BE39C);

/* Stops the sound at 0xD4. */
void func_800BE4AC(Unk800BE39C *this) {
    stopSound(this->base.unkD4);
    this->base.unkD4 = 0;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BE4DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BE71C);

/* The update of the entities of D_8006C9AC: func_800A9998, func_800BE7D0, then func_800BEA1C. */
void func_800BE788(Entity *this, s32 arg) {
    func_800A9998(this, arg);
    func_800BE7D0(this, arg);
    func_800BEA1C(this, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BE7D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BEA1C);

/* The destructor of the entity class of vtable D_8006CB44, which func_800BB034 installs. */
void func_800BED64(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CB1C, which func_800BB78C installs. */
void func_800BEDAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006CAF4 and D_8006CAD4, which
 * func_800BC870 installs. */
void func_800BEDF4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006CAD4;
    this->entity.vtable = &D_8006CAF4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CAAC, which func_800BCE94 installs. */
void func_800BEE94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA7C, which func_800BCAD8 installs. */
void func_800BEEDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA54, which func_800BD1C4 installs. */
void func_800BEF24(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA2C, which func_800BD490 installs. */
void func_800BEF6C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800BEFB4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of func_800BE39C's class: its virtual tables, then func_800A926C's destructor. */
void func_800BF054(Unk800BE39C *this, s32 flags) {
    this->base.unk18.vtable = D_8006C98C;
    this->base.entity.vtable = &D_8006C9AC;
    func_800A9428(&this->base, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006CE4C, which derives from func_80076F34's. */
Unk80076F34 *func_800BF0B4(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006CE4C;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BF0E8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BF238);

/* Whether the character's fighter wears the other colors (both sides look alike). */
s32 func_800BF37C(Unk80076F34 *this) {
    return GAME_STATE.fighters[func_80077574(this)].altColor;
}

/* Returns the first or the second word of D_8006CF74, by whether func_800BF37C gives non-zero. */
s32 func_800BF3B8(Unk80076F34 *this) {
    return D_8006CF74[func_800BF37C(this) != 0];
}

/* Returns the first or the second word of D_8006CF7C, by whether func_800BF37C gives non-zero. */
s32 func_800BF3F0(Unk80076F34 *this) {
    return D_8006CF7C[func_800BF37C(this) != 0];
}

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF238 when arg is 0. */
void func_800BF428(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 0) {
        func_800BF238(this, 2, 4, func_800BF3B8(this));
    }
}

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 0. */
void func_800BF47C(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 0) {
        func_800BF0E8(this, 2, 9, func_800BF3B8(this), 0);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BF4D4);

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF238 when arg is 0. */
void func_800BF554(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800BF238(this, 2, 10, func_800BF3B8(this));
    }
}

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 0. */
void func_800BF5A8(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 0) {
        func_800BF0E8(this, 2, 17, func_800BF3B8(this), 0);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 2. */
void func_800BF600(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 12, func_800BF3B8(this), 1);
    }
}

/* Overrides func_8007BB64 (entry 0x10C of Unk80076F34's vtable) in D_8006CE4C: calls it, then
 * func_800BF0E8 when arg is 2. */
void func_800BF660(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 16, func_800BF3B8(this), 0);
    }
}

/* Overrides func_8007BA98 (entry 0xFC of Unk80076F34's vtable) in D_8006CE4C: calls it, then
 * func_800BF0E8 when arg is 2. */
void func_800BF6BC(Unk80076F34 *this, s32 arg) {
    func_8007BA98(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 8, func_800BF3B8(this), 0);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BF718);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BF9D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BFA98);

/* Creates an entity of func_800C06D0's class (0xB0 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800BFB6C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0xB0);

    if (child != NULL) {
        func_800C06D0(entityOperatorNewAt(0xB0, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800BFBC8);

/* The override of Unk80076F34's method at 0xE8 for virtual table D_8006CE4C: func_8007BA30; arg 0 plays
 * sound 0x116, sets 0x498 to 10 and func_80077448(-0x10000); arg 4 calls func_800C0084, 5 and up
 * func_800C00EC; then spawns func_800C0210's entity and calls func_80079320 when bit 4 of 0x2C8 of
 * 0x228 is set. */
void func_800BFF28(Unk80076F34 *this, s32 arg) {
    func_8007BA30(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0x116);
        this->unk498 = 10;
        func_80077448(this, -0x10000);
    }
    if (arg == 4) {
        func_800C0084(this);
    }
    if (arg >= 5) {
        func_800C00EC(this);
    }
    func_800C0210(this);
    if (func_8006F190(func_80077580(this))) {
        func_80079320(this);
    }
}

/* The override of Unk80076F34's method at 0xF0 for virtual table D_8006CE4C: func_8007BA64; arg 0 plays
 * sound 0x116, sets 0x498 to 10 and func_80077448(-0x10000); arg 4 calls func_800C0084, 5 and up
 * func_800C00EC, lowers vy at 0x3B4 by 0x71C; then spawns func_800C0210's entity and calls func_80079320 when bit 4 of 0x2C8 of
 * 0x228 is set. */
void func_800BFFD0(Unk80076F34 *this, s32 arg) {
    func_8007BA64(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0x116);
        this->unk498 = 10;
        func_80077448(this, -0x10000);
    }
    if (arg == 4) {
        func_800C0084(this);
    }
    if (arg >= 5) {
        func_800C00EC(this);
    }
    this->unk3B4.vy -= 0x71C;
    func_800C0210(this);
    if (func_8006F190(func_80077580(this))) {
        func_80079320(this);
    }
}

/* Creates an entity of func_800C0CC0's class (0x2AC bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_800C0084(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0x2AC);
    if (child != NULL) {
        func_800C0CC0(entityOperatorNewAt(0x2AC, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C00EC);

/* Spawns the entity of func_800C62EC at the character (func_800775D8). */
void func_800C0210(Unk80076F34 *this) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &position, 0x50, 0x60, 1, 0x808080);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0298);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0344);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0434);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C06D0);

/* The destructor of Unk800C06D0 (vtable D_8006CDFC): stops its sound, takes the part out of its list,
 * then destroys the entity. */
void func_800C0830(Unk800C06D0 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006CDFC;
    stopSound(this->sound);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C08CC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0A60);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0CC0);

/* The destructor of Unk800C0CC0 (vtable D_8006CDD4): the same as func_800C0830. */
void func_800C0F10(Unk800C0CC0 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006CDD4;
    stopSound(this->sound);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C0FAC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1180);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C12A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C14F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C16F8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1758);

/* The destructor of the entity class of vtable D_8006CE4C, which func_800BF0B4 installs. */
void func_800C1840(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CE24, which func_800C0298 installs. */
void func_800C1888(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C18D0(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of the class of virtual table D_8006D044, derived from func_800BB034's. */
Unk800C1924 *func_800C1924(Unk800C1924 *this) {
    func_800BB034(&this->base);
    this->base.base.entity.vtable = &D_8006D044;
    this->unk4C4 = 0;
    return this;
}

/* The base step (func_80078910) with arg, then this class's own (func_800C1CE0). */
void func_800C195C(void *this, void *arg) {
    func_80078910(this, arg);
    func_800C1CE0(this);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1988);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1A8C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1CE0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1DBC);

/* Creates an entity of the class of vtable D_8006D01C (0x120 bytes) with g++'s new, and hands it
 * to func_8007AD4C. Its constructor func_800C27E4 is inlined. */
void func_800C1E64(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006D01C;
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C1ED8);

/* Creates an entity of func_800C2264's class (0xD4 bytes) at pos with g++'s new and hands it to func_8007AD4C, then
 * plays sound 0x11B, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800C2004(Unk80076F34 *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800C2264(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x200);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x11B);
    this->unk498 = 10;
    func_80077448(this, -0x10000);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C20A8);

extern VtableEntry D_8006CFCC[];
extern EntityVtable D_8006CFEC;

/* Constructs the class of virtual table D_8006CFEC (and D_8006CFCC for its base at 0x18) on
 * func_800BDA50's, with 0x5B05333 and 0 in its base's 0x8 and 0xC. */
Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    func_800BDA50(&this->entity, owner, pos, arg3, arg4);
    this->unk18.vtable = D_8006CFCC;
    this->entity.vtable = &D_8006CFEC;
    func_8007BDC8(&this->unk18, 0x5B05333, 0);
    return this;
}

/* The method at 0x20 of virtual table D_8006CFEC: adds func_800BE39C's and func_800C62EC's entities at 0xA4
 * as siblings, then plays sound 0x11C. */
void func_800C22C0(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB0A666, 0x500, &D_8006CFA8);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0xC0, 0xC0, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x11C, 0);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C23B8);

/* The destructor of the entity class of vtable D_8006D044, which func_800C1924 installs. */
void func_800C2754(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D01C, which func_800C1E64 and func_800C27E4 install. */
void func_800C279C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006D01C, which derives from func_800BD490's. */
Entity *func_800C27E4(Entity *this, Unk80076F34 *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006D01C;
    return this;
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800C2818(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C28B8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2CCC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2D04);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2D58);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2DF4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2E84);

/* Constructs a sprite entity: the given frame, moved from pos by vel (func_800C3078), size and color. */
Unk800C2F14 *func_800C2F14(Unk800C2F14 *this, u16 *frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color,
                           s32 flags) {
    entityInit(&this->entity, 0);
    func_800C2CCC(&this->sprite, frame, flags);
    this->entity.vtable = &D_8006D59C;
    func_800C3078(this, pos, vel, size, color);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C2FBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3078);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C30C4);

/* Has func_800C2E84 make shape of the sprite with these corners, then sets bit 3 of its flags. */
void func_800C3100(Unk800C2F14 *this, PolyFT4Shape *shape, SVECTOR *corners) {
    func_800C2E84(&this->sprite, shape, corners);
    shape->flags |= 8;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3134);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3238);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C32A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C32E0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C339C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C347C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C35D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3654);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C374C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C384C);

/* Constructs an entity of D_8006D484: func_800C3654 with entry 10 of D_8006D5C4. */
Entity *func_800C38D8(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    func_800C3654(this, arg1, arg2, arg3, arg4, 10);
    this->vtable = &D_8006D484;
    return this;
}

/* Constructs an entity of D_8006D45C: func_800C3654 with entry 11 of D_8006D5C4. */
Entity *func_800C391C(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    func_800C3654(this, arg1, arg2, arg3, arg4, 11);
    this->vtable = &D_8006D45C;
    return this;
}

extern EntityVtable D_8006D434;

/* The entity of func_800C3960 (vtable D_8006D434, 0xE4 bytes) for a character, by its handle. */
typedef struct Unk800C3960 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 chara; /* the character's handle */
    /* 0x1C */ u8 unk1C[0xDC - 0x1C];
    /* 0xDC */ s32 unkDC; /* whether the character's func_80079E2C is 15 */
    /* 0xE0 */ s32 unkE0;
} Unk800C3960; /* size 0xE4 */

/* Constructs the character's entity of vtable D_8006D434. */
Unk800C3960 *func_800C3960(Unk800C3960 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D434;
    this->chara = entityGetHandle(&chara->entity);
    this->unkE0 = 0;
    this->unkDC = func_80079E2C(chara) == 15;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C39C8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3B40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3CFC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3E64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C3ED0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4204);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C42D0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4480);

/* The entity class of virtual table D_8006D3BC (constructor func_800C4860). */
typedef struct Unk800C4860 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18; /* unk1C + unk24 */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
} Unk800C4860;

extern EntityVtable D_8006D3BC;

/* Constructs the entity of virtual table D_8006D3BC with its three values, and the sum of the first and
 * the third at 0x18. */
Unk800C4860 *func_800C4860(Unk800C4860 *this, s32 arg1, s32 arg2, s32 arg3) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D3BC;
    this->unk18 = arg1 + arg3;
    this->unk1C = arg1;
    this->unk20 = arg2;
    this->unk24 = arg3;
    this->unk28 = 0;
    this->unk2C = 0;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C48D8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4940);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C49E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4ADC);

/* Spawns the entity of func_800C3CFC at the position, as a sibling. */
void func_800C4CD8(Unk800C49E4 *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_800C3CFC(entityOperatorNewAt(0x54, child), &this->position, 0x280, 0x403020);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of func_800C62EC at the position (radius 0x60, colour 0x8090A0), as a sibling. */
void func_800C4D40(Unk800C49E4 *this, s32 arg1, s32 count) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), arg1, &this->position, 0x60, 0x20, count, 0x8090A0);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4DD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C4F74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C52C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5480);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5680);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5778);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5834);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C59D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5B18);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5B84);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5BD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5C4C);

/* The entity class of virtual table D_8006D314 (constructor func_800C5DF4): two vectors and three values. */
typedef struct Unk800C5DF4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 unk2C;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Unk800C5DF4;

extern EntityVtable D_8006D314;

/* Constructs the entity of virtual table D_8006D314 with arg1, copies of the two vectors and three values.
 * The match depends on reading each vector's three words before storing any, as vecCopy does. */
Unk800C5DF4 *func_800C5DF4(Unk800C5DF4 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2, s32 arg4, s32 arg5, s32 arg6) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D314;
    this->unk18 = arg1;
    x = vec1->vx;
    y = vec1->vy;
    z = vec1->vz;
    this->unk1C.vx = x;
    this->unk1C.vy = y;
    this->unk1C.vz = z;
    x2 = vec2->vx;
    y2 = vec2->vy;
    z2 = vec2->vz;
    this->unk2C.vx = x2;
    this->unk2C.vy = y2;
    this->unk2C.vz = z2;
    this->unk3C = arg4;
    this->unk40 = arg5;
    this->unk44 = arg6;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C5EB4);

/* Constructs an Unk800C6088 (vtable D_8006D2EC): count particles around pos. */
Unk800C6088 *func_800C6088(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D2EC;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C6134);

/* Constructs an Unk800C6088 (vtable D_8006D2C4): count particles around pos. */
Unk800C6088 *func_800C62EC(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D2C4;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C6398);

/* Constructs an Unk800C6088 (vtable D_8006D29C): count particles around pos. */
Unk800C6088 *func_800C652C(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D29C;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C65D8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C67C0);

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C67CC(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C6820);

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6880(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D574: func_800C2FBC with the arguments and 0. */
Entity *func_800C68D4(Entity *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    func_800C2FBC((Unk800C2F14 *)this, frame, pos, vel, size, color, 0);
    this->vtable = &D_8006D574;
    return this;
}

/* Constructs an entity of D_8006D574: func_800C2FBC with frame, color 0x808080 and no flags. */
Entity *func_800C691C(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D574;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6970(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

extern EntityVtable D_8006D54C;

/* Constructs an entity of D_8006D54C: func_800C2FBC with frame + 3, color 0x808080 and no flags. */
Entity *func_800C69C4(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame + 3, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D54C;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6A20(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C6A74);

/* A copy of the destructor of the entity class of vtable D_8006D4FC, which func_800C32E0
 * installs: game has 2 identical ones. */
void func_800C6AC8(Entity *this, s32 flags) {
    this->vtable = &D_8006D4FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6B1C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C6B70);

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6BC0(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6C14(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6C68(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D434, which func_800C3960 installs. */
void func_800C6CBC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D40C, which func_800C3CFC installs. */
void func_800C6D04(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D3E4, which func_800C4204 installs:
 * its sprite, then the entity. */
void func_800C6D4C(Unk800C4204 *this, s32 flags) {
    spriteDestroy(&this->sprite, DESTROY_BASES);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D3BC, which func_800C4860 installs. */
void func_800C6DA4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D394, which func_800C49E4 installs. */
void func_800C6DEC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D36C, which func_800C4F74 installs. */
void func_800C6E34(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D33C, which func_800C5680 installs. */
void func_800C6E7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* When 0x24 is above 0, runs func_800C5834 and func_800C59D4. */
void func_800C6EC4(Unk800C6EC4 *this) {
    if (this->unk24 > 0) {
        func_800C5834(this);
        func_800C59D4(this);
    }
}

/* The destructor of the entity class of vtable D_8006D314, which func_800C5DF4 installs. */
void func_800C6F04(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D2EC, which func_800C6088 installs. */
void func_800C6F4C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D2C4, which func_800C62EC installs. */
void func_800C6F94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D29C, which func_800C652C installs. */
void func_800C6FDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7024);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C71DC);

s32 func_800C7358(Unk800C7024 *this);
void func_800C73A8(Unk800C7024 *this, s32 arg);
void func_800C7474(Unk800C7024 *this, Entity *target);

/* The second base's first method of Unk800C7024: func_8007BDF0 with sum, func_800C73A8 with what func_800C7358
 * picks, func_800C7474 with the entity func_80078878 finds in sum, then deletes itself. */
void func_800C72D8(Unk800C7024 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->parts.base, sum);
    func_800C73A8(this, func_800C7358(this));
    func_800C7474(this, func_80078878(sum));
    entityDelete(&this->parts.entity);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7358);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C73A8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7474);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C75FC);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800C7714(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800C77B4(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C77BC);

/* Creates func_800C82CC's entity (0x3C bytes) with the two arguments and adds it to the group at
 * 0x3C (the stages call it with a table of their .data). */
void func_800C77C8(Unk8006ED20 *this, void *arg1, s32 arg2) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C82CC(entityOperatorNewAt(0x3C, child), this, arg1, arg2);
    }
    func_8006F084(this, child);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7844);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7924);

/* Creates an entity of func_800CA430's class (0x24 bytes) with g++'s new, and hands it to
 * func_8007AD4C. */
void func_800C7B30(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x24);

    if (child != NULL) {
        func_800CA430(entityOperatorNewAt(0x24, child), this, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800CA768's class (0x30 bytes) with g++'s new, and hands it to
 * func_8007AD4C. */
void func_800C7B9C(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        func_800CA768(entityOperatorNewAt(0x30, child), this, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800CAACC's entity (0x48 bytes) for the character with the four arguments, and hands
 * it to func_8007AD4C. */
void func_800C7C08(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_800CAACC(entityOperatorNewAt(0x48, child), this, arg1, arg2, arg3, arg4);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800CAEF0's entity (0x54 bytes) at the character's func_8007A650 point with 0x100 and
 * the colour (0x2020FF, 0xFF2020), and hands it to func_8007AD4C. arg1 is not read. */
void func_800C7C9C(Unk80076F34 *this, s32 arg1, s32 color) {
    Vec3 position;
    Entity *child;

    func_8007A650(this, &position);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800CAEF0(entityOperatorNewAt(0x54, child), &position, 0x100, color);
    }
    func_8007AD4C(this, child);
}

/* Throws count func_800C3654 entities (entry 9 of D_8006D5C4) from the character's position (func_800775D8),
 * each 256 either side of it in x, at a random velocity (x -8 to 7, y -50 to -19) from the character's
 * generator (func_80077580), with color. */
void func_800C7D14(Unk80076F34 *this, s32 arg1, u32 count, u32 color) {
    Unk8006ED20 *random;
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    random = func_80077580(this);
    while (count--) {
        func_800775D8(this, &position);
        position.vx += (func_8006F004(random) & 0x1FF) - 0x100;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = func_8006F004(random) & 0xF;
        vy = func_8006F004(random) & 0x1F;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x32;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            func_800C3654(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color, 9);
        }
        func_8007AD4C(this, child);
    }
}

/* Throws count func_800C391C entities (entry 11 of D_8006D5C4) from the character's point (func_8007A650), each
 * 16 either side of it in x, at a random velocity (x -8 to 7, y -20 to -13), with color. */
void func_800C7E1C(Unk80076F34 *this, s32 arg1, u32 count, u32 color) {
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    while (count--) {
        func_8007A650(this, &position);
        position.vx += (mersenneTwisterGenerate(&RANDOM) & 0x1F) - 0x10;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = mersenneTwisterGenerate(&RANDOM) & 0xF;
        vy = mersenneTwisterGenerate(&RANDOM) & 7;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x14;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            func_800C391C(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color);
        }
        func_8007AD4C(this, child);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7F14);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C7F48);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8008);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C811C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C82CC);

/* The destructor of the entity class of vtable D_8006DBA0, which func_800C82CC installs. */
void func_800C8380(Entity *this, s32 flags) {
    this->vtable = &D_8006DBA0;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C83D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C84C8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8520);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C85C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8658);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C86E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8718);

/* The first method of Unk800C8778: func_800C8DFC on the object at 4, with arg. */
void func_800C8754(Unk800C8778 *this, s32 arg) {
    func_800C8DFC(this->unk4, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8778);

extern EntityVtable D_8006DB50;

/* The destructor of the entity class of vtable D_8006DB50, which func_800C8778 installs. */
void func_800C8920(Unk800C90FC *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006DB50;
    /* the members' destructors, inlined in reverse order: the one at 0x60 first */
    this->unk60.base.vtable = D_80065164;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C89B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8B0C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8C40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8CD0);

/* Calls the object's virtual method at 0x20 with arg. */
void func_800C8DFC(Unk800C8DFC *object, s32 arg) {
    object->vtable->unk20.func((u8 *)object + object->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8E30);

/* func_8002BD68 of the entity at 0x14 of arg, or 0 when there is none. */
s32 func_800C8F04(void *this, Unk800C8F04 *arg) {
    s32 result = 0;

    if (arg->unk14) {
        result = func_8002BD68(arg->unk14);
    }
    return result;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8F34);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8F7C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C8F9C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9034);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9080);
void func_800C9080(Entity *this, u8 bits);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C90C0);

/* The update of the four classes derived from Unk800C90FC: func_800C9288 while 0x68 is under 11, func_800C9140 after. */
void func_800C90FC(Unk800C90FC *this, s32 arg) {
    if (this->unk68 < 11) {
        func_800C9288(this, arg);
    } else {
        func_800C9140(this, arg);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9140);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9288);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C94C4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9790);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9938);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C99B0);

/* Constructs an entity of D_8006DB20: func_800C8778 with the same arguments. */
Entity *func_800C9A88(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DB20;
    return this;
}

/* The first method of the class of virtual table D_8006DB20: func_800C89B4. */
void func_800C9AD4(void *this) {
    func_800C89B4(this);
}

/* The entity class of virtual table D_8006DAF0 (func_800C9AF4), derived from func_800C8778's. */
typedef struct Unk800C9AF4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0xB8 - 0x18];
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ u8 unkBC[0xC4 - 0xBC];
    /* 0xC4 */ s32 unkC4;
} Unk800C9AF4;

/* Constructs an entity of D_8006DAF0: func_800C8778 with the same arguments, 0xB8 moved by 0x800, then
 * func_800C9080 with bit 1. */
Unk800C9AF4 *func_800C9AF4(Unk800C9AF4 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(&this->entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->entity.vtable = &D_8006DAF0;
    this->unkC4 = 0;
    this->unkB8 += 0x800;
    func_800C9080(&this->entity, 2);
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9B5C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9BA4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9C58);

/* Constructs an entity of D_8006DAC0: func_800C8778 with the same arguments. */
Entity *func_800C9CA0(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DAC0;
    return this;
}

/* The first method of the class of virtual table D_8006DAC0: func_800C89B4. */
void func_800C9CEC(void *this) {
    func_800C89B4(this);
}

/* The entity class of virtual table D_8006DA90 (func_800C9D0C), derived from func_800C8778's: four 0x24-byte
 * records from 0xC8. */
typedef struct Unk800C9D0CItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 unk2[0x24 - 0x2];
} Unk800C9D0CItem;

typedef struct Unk800C9D0C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0xC4 - 0x18];
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ Unk800C9D0CItem unkC8[4];
} Unk800C9D0C;

/* Constructs an entity of D_8006DA90: func_800C8778 with the same arguments, then clears the first halfword of
 * its four records, the last first. */
Unk800C9D0C *func_800C9D0C(Unk800C9D0C *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    s32 i;

    func_800C8778(&this->entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->entity.vtable = &D_8006DA90;
    this->unkC4 = 0;
    /* The match depends on the loop running from the last record (the game clears 0x134 first). */
    for (i = 3; i >= 0; i--) {
        this->unkC8[i].unk0 = 0;
    }
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800C9D74);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA028);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA1B4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA430);

/* The destructor of the entity class of vtable D_8006DA68, which func_800CA430 installs. */
void func_800CA4A8(Entity *this, s32 flags) {
    this->vtable = &D_8006DA68;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA4FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA5AC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA768);

/* The destructor of the entity class of vtable D_8006DA40, which func_800CA768 installs. */
void func_800CA7F8(Entity *this, s32 flags) {
    this->vtable = &D_8006DA40;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA84C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CA920);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CAACC);

/* The destructor of the entity class of vtable D_8006DA18, which func_800CAACC installs. */
void func_800CAB94(Entity *this, s32 flags) {
    this->vtable = &D_8006DA18;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CABE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CACB4);

/* Constructs an entity of the class of virtual table D_8006D9F0, derived from func_800C3CFC's at pos. */
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3) {
    func_800C3CFC(this, pos, arg2, arg3);
    this->entity.vtable = &D_8006D9F0;
    this->unk50 = 8;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CAF2C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CAFC4);

/* func_800C72D8 for a derived class of Unk800C7024: only when func_80078878 finds an entity in sum, which also
 * gets func_800773F4 with 50 on its character (func_8007AEEC). */
void func_800CB010(Unk800C7024 *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->parts.base, sum);
    func_800C73A8(this, func_800C7358(this));
    target = func_80078878(sum);
    if (target != NULL) {
        func_800773F4(func_8007AEEC((Unk80076F34 *)target), 50);
        func_800C7474(this, target);
    }
    entityDelete(&this->parts.entity);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB0B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB25C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB338);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB3CC);

/* The destructor of the class of virtual table D_8006DB80: Unk80076308's, inlined. */
void func_800CB508(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an object of the class of vtable D_8006DB80, which derives from the class of
 * D_80065164, with a word. */
Unk800CB53C *func_800CB53C(Unk800CB53C *this, s32 arg) {
    this->vtable = D_8006DB80;
    this->unk4 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006DB20, which func_800C9A88 installs. */
void func_800CB554(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAF0, which func_800C9AF4 installs. */
void func_800CB59C(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAC0, which func_800C9CA0 installs. */
void func_800CB5E4(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DA90, which func_800C9D0C installs. */
void func_800CB62C(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D9F0, which func_800CAEF0 installs. */
void func_800CB674(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800CB6BC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800CB75C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB7B0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CB988);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBA04);

/* The start of the entity class func_800CB7B0 constructs (0x59C bytes): two frame counters it zeroes, and
 * at 0x58C and 0x594 the pointers to member functions of its states (tables D_800D36A4 and D_800D370C). */
typedef struct Unk800CB7B0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x30 - 0x18];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
} Unk800CB7B0;

/* A state of D_800D36A4 (entries 1 and 10): counts both frame counters, and deletes the entity after 91
 * frames. */
void func_800CBA80(Unk800CB7B0 *this) {
    this->unk30++;
    this->unk3C++;
    if (this->unk3C >= 91) {
        entityDelete(&this->entity);
    }
}

/* The object whose member functions D_800D370C points to (func_800CBAE0 draws its text). */
typedef struct Unk800CBAE0 {
    /* 0x00 */ u8 unk0[0x3C];
    /* 0x3C */ s32 unk3C;
} Unk800CBAE0;

void func_800CBAE0(Unk800CBAE0 *this, u_long *ot);
void func_800CF2A4(u_long *ot, s32 brightness);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBAE0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBB88);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBBD4);

/* A member function (its pointer is in D_800D370C): func_800CBAE0, then func_800CF2A4 in the next slot
 * of ot with 90 minus 0x3C. */
void func_800CBE4C(Unk800CBAE0 *this, u_long *ot) {
    func_800CBAE0(this, ot);
    func_800CF2A4(ot + 1, 90 - this->unk3C);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBE90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBF6C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CBFF0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CC230);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CC3E4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CC6DC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CC750);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CC93C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CCA90);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CCC0C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CCE18);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD068);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD0F4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD140);

/* Sets every character's unk0 and unk2. */
void func_800CD210(Unk800CD140 *this, s32 unk0, s32 unk2) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk0 = unk0;
        this->chars[i].unk2 = unk2;
    }
}

/* Sets unk0 and unk2 of count characters from start on (as far as the text goes). */
void func_800CD248(Unk800CD140 *this, s32 arg1, s32 arg2, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unk0 = arg1;
        this->chars[i].unk2 = arg2;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD29C);

/* Sets every character's unkC. */
void func_800CD324(Unk800CD140 *this, s32 unkC) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unkC = unkC;
    }
}

/* Sets unkC of count characters from start on (as far as the text goes). */
void func_800CD358(Unk800CD140 *this, s32 value, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unkC = value;
    }
}

/* Sets every character's unk4 and unk6 to the same value. */
void func_800CD3A8(Unk800CD140 *this, s32 value) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk4 = value;
        this->chars[i].unk6 = value;
    }
}

/* Sets every character's unk4 and unk6. */
void func_800CD3E0(Unk800CD140 *this, s32 unk4, s32 unk6) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk4 = unk4;
        this->chars[i].unk6 = unk6;
    }
}

/* Sets unk4 and unk6 of count characters from start on (as far as the text goes). */
void func_800CD418(Unk800CD140 *this, s32 arg1, s32 arg2, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unk4 = arg1;
        this->chars[i].unk6 = arg2;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD46C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD474);

/* Sets every character's color cd byte. */
void func_800CD4B0(Unk800CD140 *this, s32 value) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].color.cd = value;
    }
}

/* Sets the color's cd byte of count characters from start on (as far as the text goes). */
void func_800CD4E4(Unk800CD140 *this, s32 value, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].color.cd = value;
    }
}

/* Sets every character's unk10, adding step for each next one. */
void func_800CD534(Unk800CD140 *this, s32 unk10, s32 step) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk10 = unk10;
        unk10 += step;
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD56C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD6D4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD754);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CD8C8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CDA20);

/* The destructor of the entity class of vtable D_8006DD6C, which func_800CDA20 installs:
 * func_800CDFBC first, then the entity. */
void func_800CDB78(Entity *this, s32 flags) {
    this->vtable = &D_8006DD6C;
    func_800CDFBC(this);
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CDBD4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CDF14);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CDFBC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CDFEC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CE240);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CE388);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CEA40);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CEB0C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CED10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CEEDC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF0A0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF124);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF2A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF460);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF528);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF6F0);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CF994);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CFB3C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CFD10);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800CFE48);

/* The destructor of the entity class of vtable D_8006DDBC, which func_800CB7B0 installs. */
void func_800CFFB4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DD94, which func_800CD6D4 installs. */
void func_800CFFFC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DD44, which func_800CFB3C installs. */
void func_800D0044(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Adds a DR_TPAGE (tpage 30) to the ordering table slot. */
void func_800D008C(u_long *ot) {
    AddPrim(ot, allocDrTpageMode(0, 0, 30));
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D00C8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0204);

/* Constructs an object of the class of vtable D_8006DF3C, which derives from loadingScreenInit's. */
LoadingScreen *func_800D0294(LoadingScreen *this) {
    loadingScreenInit(this);
    this->vtable = &D_8006DF3C;
    return this;
}

/* Constructs an object of the class of vtable D_8006E154, which derives from func_8006ED20's. */
Unk8006ED20 *func_800D02C8(Unk8006ED20 *this) {
    func_8006ED20(this);
    this->vtable = D_8006E154;
    return this;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D02FC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0364);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0504);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D06D0);

/* Stops the sound at 0xDA4. */
void func_800D07FC(Unk800D02C8 *this) {
    stopSound(this->unkDA4);
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0820);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D098C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0ABC);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0B80);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0C50);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0C9C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0CF8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0F1C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D0F7C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D110C);

/* Constructs an entity of the class of virtual table D_8006E12C, for func_8006ED20's object. */
Unk800D1328 *func_800D1328(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E12C;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E12C, which func_800D1328 installs. */
void func_800D1370(Entity *this, s32 flags) {
    this->vtable = &D_8006E12C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D13C4);

/* Constructs an entity of the class of virtual table D_8006E104, for func_8006ED20's object. */
Unk800D1328 *func_800D17E4(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E104;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E104, which func_800D17E4 installs. */
void func_800D182C(Entity *this, s32 flags) {
    this->vtable = &D_8006E104;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D1880);

/* Constructs an entity of the class of virtual table D_8006E0B4, for func_8006ED20's object. */
Unk800D1C50 *func_800D1C50(Unk800D1C50 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E0B4;
    this->unk18 = arg;
    this->unk1C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006E0B4, which func_800D1C50 installs. */
void func_800D1C9C(Entity *this, s32 flags) {
    this->vtable = &D_8006E0B4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Adds 1 to 0x1C each time its group gathers it (the gather of D_8006E0B4). */
void func_800D1CF0(Unk800D1C50 *this) {
    this->unk1C++;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D1D04);

/* Constructs an Unk800D1F8C (vtable D_8006E08C) of owner: copies the 16-color palettes at x 640, y 245 and 244 of
 * VRAM. */
Unk800D1F8C *func_800D1F8C(Unk800D1F8C *this, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E08C;
    this->owner = owner;
    this->unk68 = 0;
    this->clutRects[0].x = 0x280;
    this->clutRects[0].y = 0xF5;
    this->clutRects[0].w = 0x10;
    this->clutRects[0].h = 1;
    func_80055B88(&this->clutRects[0], (u_long *)this->cluts[0]);
    this->clutRects[1].x = 0x280;
    this->clutRects[1].y = 0xF4;
    this->clutRects[1].w = 0x10;
    this->clutRects[1].h = 1;
    func_80055B88(&this->clutRects[1], (u_long *)this->cluts[1]);
    return this;
}

/* The destructor of the entity class of vtable D_8006E08C, which func_800D1F8C installs. */
void func_800D2034(Entity *this, s32 flags) {
    this->vtable = &D_8006E08C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The object of the setters func_800D3284-func_800D32AC (and of func_800D326C, which reads 0xD98), none of them
 * called. Probably func_800D02C8's class (the stage base of st07-st09: their stage objects have 0xF34 and
 * 0xF6A too); func_800D24C0 prints 0xF30 and 0xF32 for its owner, by side. */
typedef struct Unk800D3284 {
    /* 0x000 */ u8 unk0[0xD98];
    /* 0xD98 */ s16 unkD98[(0xF2C - 0xD98) / 2];
    /* 0xF2C */ s16 unkF2C[2];
    /* 0xF30 */ s16 unkF30[2];
    /* 0xF34 */ s16 unkF34;
    /* 0xF36 */ s16 unkF36;
    /* 0xF38 */ u8 unkF38[0xF64 - 0xF38];
    /* 0xF64 */ u16 unkF64[2]; /* by side: func_800D2088 prints them and flashes the higher one's palette */
    /* 0xF68 */ u8 unkF68[0xF6A - 0xF68];
    /* 0xF6A */ u16 unkF6A; /* func_800D2088 and func_800D24C0 draw nothing while it is set */
} Unk800D3284;

/* The owner of an Unk800D1F8C as the stage it is (game_classes.h types it Unk8006ED20). */
static inline Unk800D3284 *paletteOwner(Unk800D1F8C *this) {
    return (Unk800D3284 *)this->owner;
}

/* A TextPrinter's virtual calls, inlined as the game has them: printerSetColor and func_80032900 on the
 * printer object itself (its virtual table read from the stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)
#define TEXT_SET_UNK18(printer, value) \
    (printer).printer.vtable->unk18.func((u8 *)&(printer) + (printer).printer.vtable->unk18.delta, value)

/* The update of D_8006E08C's entity: unless the owner's 0xF6A is set, prints the two sides' 0xF64 counts
 * (shadowed); if 0xF6A got set meanwhile, pulses entries 7 and 8 of the leading side's palette; then draws two
 * 120 by 24 sprites of the palettes. */
void func_800D2088(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->unkF6A != 0) {
        return;
    }
    ot = camera->ot.slots;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 77;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[0]);
    printer.printer.x = 236;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 78;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[0]);
    printer.printer.x = 237;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[1]);

    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->unkF6A != 0) {
        /* The match depends on the operand order of both compares (0xF64 loads first, then 0xF66). */
        if (paletteOwner(this)->unkF64[0] > paletteOwner(this)->unkF64[1]) {
            this->cluts[0][7] = color;
            this->cluts[0][8] = brightColor;
            func_80055B28(&this->clutRects[0], (u_long *)this->cluts[0]);
        }
        if (paletteOwner(this)->unkF64[1] > paletteOwner(this)->unkF64[0]) {
            this->cluts[1][7] = color;
            this->cluts[1][8] = brightColor;
            func_80055B28(&this->clutRects[1], (u_long *)this->cluts[1]);
        }
    }

    tpage = allocDrTpage(10);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 20;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3D68;
    sprt->w = 120;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 180;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3D28;
    sprt->w = 120;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    this->unk68++;
    /* The match depends on the destructor taking the printer through a variable set here: gcse then reuses
     * the first &printer in a second register (fp), as g++ passes a local's address to its destructor. */
    base = &printer.printer;
    printerDestroy(base, DESTROY_BASES);
}

/* Constructs an Unk800D1F8C (vtable D_8006E064) of owner: copies the 16-color palettes at x 640, y 250 and 248 of
 * VRAM. */
Unk800D1F8C *func_800D23C4(Unk800D1F8C *this, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E064;
    this->owner = owner;
    this->unk68 = 0;
    this->clutRects[0].x = 0x280;
    this->clutRects[0].y = 0xFA;
    this->clutRects[0].w = 0x10;
    this->clutRects[0].h = 1;
    func_80055B88(&this->clutRects[0], (u_long *)this->cluts[0]);
    this->clutRects[1].x = 0x280;
    this->clutRects[1].y = 0xF8;
    this->clutRects[1].w = 0x10;
    this->clutRects[1].h = 1;
    func_80055B88(&this->clutRects[1], (u_long *)this->cluts[1]);
    return this;
}

/* The destructor of the entity class of vtable D_8006E064, which func_800D23C4 installs. */
void func_800D246C(Entity *this, s32 flags) {
    this->vtable = &D_8006E064;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of D_8006E064's entity: unless the owner's 0xF6A is set, prints the two sides' 0xF30 counts
 * (shadowed), pulses entries 7 and 8 of each palette (the side's count still 0) and draws two 96 by 24
 * sprites of the palettes. */
void func_800D24C0(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->unkF6A != 0) {
        return;
    }
    ot = camera->ot.head;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 90;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[0]);
    printer.printer.x = 250;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 89;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[0]);
    printer.printer.x = 249;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[1]);

    /* The match depends on green and blue in their own variables (the bright color adds 4 to each). */
    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->unkF30[0] == 0) {
        this->cluts[0][7] = color;
        this->cluts[0][8] = brightColor;
        func_80055B28(&this->clutRects[0], (u_long *)this->cluts[0]);
    }
    if (paletteOwner(this)->unkF30[1] == 0) {
        this->cluts[1][7] = color;
        this->cluts[1][8] = brightColor;
        func_80055B28(&this->clutRects[1], (u_long *)this->cluts[1]);
    }

    tpage = allocDrTpage(10);
    /* The match depends on this field order, which sets the sprite registers. */
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 32;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3EA8;
    sprt->w = 96;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 192;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3E28;
    sprt->w = 96;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    this->unk68++;
    base = &printer.printer; /* as in func_800D2088 */
    printerDestroy(base, DESTROY_BASES);
}

/* Constructs an entity of the class of virtual table D_8006E0DC, for func_8006ED20's object. */
Unk800D1328 *func_800D27E0(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E0DC;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E0DC, which func_800D27E0 installs. */
void func_800D2828(Entity *this, s32 flags) {
    this->vtable = &D_8006E0DC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D287C);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D29C8);

/* The destructor of the entity class of vtable D_8006E03C, which func_800D29C8 installs. */
void func_800D2B94(Entity *this, s32 flags) {
    this->vtable = &D_8006E03C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D2BE8);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D2C64);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D2E98);

/* The destructor of the entity class of vtable D_8006E014, which func_800D2E98 installs. */
void func_800D2F8C(Entity *this, s32 flags) {
    this->vtable = &D_8006E014;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D2FE0(void) {
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D2FE8);

/* Returns entry index of the array at 0xD98; nothing calls it. */
s16 func_800D326C(Unk800D3284 *this, u16 index) {
    return this->unkD98[index];
}

/* Sets entry index of the array at 0xF2C; nothing calls it. */
void func_800D3284(Unk800D3284 *this, s32 index, s32 value) {
    this->unkF2C[index] = value;
}

/* Sets entry index of the array at 0xF30; nothing calls it. */
void func_800D3294(Unk800D3284 *this, s32 index, s32 value) {
    this->unkF30[index] = value;
}

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D32A4);

INCLUDE_ASM("asm/jp/game/nonmatchings/game", func_800D32AC);
