#ifndef DTBE_MOVIE_MOVIE_H
#define DTBE_MOVIE_MOVIE_H

/* The movie scene: a task that plays an STR movie from the disc (libcd's streaming, libpress's MDEC decoding) until it ends or a button skips it. */

#include "common.h"
#include <sys/types.h>
#include <libcd.h>
#include <libgpu.h>
#include <libpress.h>
#include "engine/task/task.h"

/* the movie the scene plays, and its last frame */
#define MOVIE_FILE "/a.str"
#define MOVIE_END_FRAME 1350

/* the buttons that skip the movie */
#define MOVIE_SKIP_BUTTONS (PADstart | PADRup | PADRright | PADRdown | PADRleft)

/* how many sectors the streaming ring holds (StSetRing), and their size */
#define MOVIE_RING_SECTORS 32
#define MOVIE_SECTOR_SIZE 2048

/* how many times the next frame is asked for before giving up */
#define MOVIE_FRAME_TRIES 2000
/* how many times sync checks the frame before it gives it up */
#define MOVIE_SYNC_TRIES 0x800000

#ifdef __cplusplus

/* What a MoviePlayer plays and where: the file, its frames, the color depth
 * and the place on the screen. */
class MovieParams {
public:
    /* 0x00 */ s32 rgb24;      /* 24-bit color, else 15-bit */
    /* 0x04 */ s32 startFrame; /* -1 if the file is not on the disc */
    /* 0x08 */ s32 endFrame;   /* the frame it ends at; -1 if the file is not on the disc */
    /* 0x0C */ s32 x;          /* where the movie is on the screen */
    /* 0x10 */ s32 y;
    /* 0x14 */ s32 width;      /* the size of the screen */
    /* 0x18 */ s32 height;
    /* 0x1C */ CdlFILE file;

    MovieParams(char *name, s32 startFrame, s32 endFrame) __asm__("movieParamsInit");
    void setRgb24(s32 rgb24) __asm__("movieParamsSetRgb24");
    void setPosition(s32 x, s32 y) __asm__("movieParamsSetPosition");
    void setSize(s32 width, s32 height) __asm__("movieParamsSetSize");
    s32 isValid() __asm__("movieParamsIsValid");
}; /* size 0x34 */

/* Plays an STR movie: streams its sectors from the disc into a ring, decodes
 * a frame's VLC while the MDEC decodes the last one, slice by slice, into one
 * of two places in VRAM, and shows the other. */
class MoviePlayer {
public:
    /* 0x00000 */ MovieParams params;
    /* 0x00034 */ s32 vlcId;      /* the vlcBuffers entry decoded last */
    /* 0x00038 */ s32 imageId;    /* the imageBuffers entry the MDEC decodes into */
    /* 0x0003C */ RECT rects[2];  /* the two places in VRAM the frames are decoded to */
    /* 0x0004C */ s32 rectId;     /* the rects entry being decoded to */
    /* 0x00050 */ RECT slice;     /* the slice of the frame the MDEC decodes */
    /* 0x00058 */ s32 frameWidth; /* the size of the stream's frames */
    /* 0x0005C */ s32 frameHeight;
    /* 0x00060 */ bool decoded; /* the frame is decoded */
    /* 0x00064 */ s32 unk64;       /* serve the CD's interrupts in the decoding callback */
    /* 0x00068 */ bool firstSlice; /* the next slice is the first of its frame */
    /* 0x0006C */ bool finished;   /* the end frame was read */
    /* 0x00070 */ bool started;
    /* 0x00074 */ bool streaming;
    /* 0x00078 */ CdlLOC loc;     /* where the stream is read from */
    /* 0x0007C */ DISPENV dispEnv; /* the display the frames are shown in */
    /* 0x00090 */ u_long vlcBuffers[2][0xA000];   /* the frames' VLC, decoded */
    /* 0x50090 */ u_long imageBuffers[2][0x1E00]; /* the slices the MDEC decodes */
    /* 0x5F090 */ DECDCTTAB vlcTable;
    /* 0x70090 */ u_long ring[MOVIE_RING_SECTORS * MOVIE_SECTOR_SIZE / sizeof(u_long)];
    /* 0x80090: the virtual table pointer */

    MoviePlayer(MovieParams *source) __asm__("moviePlayerInit");
    /* called after a frame is decoded */
    virtual void afterDecode() __asm__("moviePlayerAfterDecode");
    /* called with the display a frame is shown in */
    virtual void beforeShow(DISPENV *dispEnv) __asm__("moviePlayerBeforeShow");
    virtual ~MoviePlayer();
    void stop() __asm__("moviePlayerStop");
    DISPENV *getDispEnv() __asm__("moviePlayerGetDispEnv");
    void start() __asm__("moviePlayerStart");
    void update() __asm__("moviePlayerUpdate");
    s32 isFinished() __asm__("moviePlayerIsFinished");
    void clearScreen() __asm__("moviePlayerClearScreen");
    void openStream(CdlLOC *loc) __asm__("moviePlayerOpenStream");
    void loadSlice() __asm__("moviePlayerLoadSlice");
    s32 decodeVlc() __asm__("moviePlayerDecodeVlc");
    u_long *getNextFrame() __asm__("moviePlayerGetNextFrame");
    void sync() __asm__("moviePlayerSync");
}; /* size 0x80094 */

/* The movie scene's task. */
class MovieTask : public Task {
public:
    /* 0x20 */ MoviePlayer *player;
    /* 0x24 */ s32 frames; /* the frames played; -1 once the movie is over */

    MovieTask() __asm__("movieTaskInit");
    ~MovieTask();
    void update(s32 arg) __asm__("movieTaskUpdate");
}; /* size 0x28 */

#endif /* __cplusplus */

EXTERN_C_BEGIN

/* the player the decoding callback works for */
extern struct MoviePlayer *MOVIE_PLAYER;

struct MoviePlayer *createMoviePlayer(char *name, s32 x, s32 y, s32 endFrame);
void movieReadStream(CdlLOC *loc);
void movieSliceDecoded(void);

/* DsRead2: starts reading from pos in a mode */
s32 func_80045820(CdlLOC *pos, s32 mode);
/* DsControlB: issues a command to the drive and waits for it */
s32 func_80046D50(u8 command, u8 *param, u8 *result);
/* StCdIntrFlag: a CD interrupt came while the MDEC was busy */
extern s32 D_80116914;

EXTERN_C_END

#endif /* DTBE_MOVIE_MOVIE_H */
