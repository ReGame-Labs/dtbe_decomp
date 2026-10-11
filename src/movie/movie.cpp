#include "common.h"
#include "movie/movie.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/gfx/display.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/task/task.h"
#include "libetc.h"
#include "psyq.h"

/* Constructs the movie scene: the player of /a.str, shown instead of the
 * display's buffers. */
MovieTask::MovieTask() : Task(0, "no name") {
    MoviePlayer *player = createMoviePlayer(MOVIE_FILE, 0, 0, MOVIE_END_FRAME);
    Display *display;

    soundControlFadeIn(&SOUND_CONTROL, 0);
    /* The match depends on reading the display before asking the player for
     * its DISPENV: the game keeps it in s0 across that call */
    display = CURRENT_DISPLAY;
    displaySetOverride(display, player->getDispEnv());
    this->player = player;
    frames = 0;
}

/* Destroys the movie scene and its player, and gives the display its buffers back. */
MovieTask::~MovieTask() {
    delete player;
    displaySetOverride(CURRENT_DISPLAY, NULL);
}

/* Makes a player for the frames of a file up to endFrame, at x, y on the
 * screen; NULL if the file is not on the disc. */
MoviePlayer *createMoviePlayer(char *name, s32 x, s32 y, s32 endFrame) {
    MovieParams params(name, 1, endFrame);

    if (!params.isValid()) {
        return NULL;
    }
    params.setPosition(x, y);
    return new MoviePlayer(&params);
}

/* Plays a frame of the movie; ends the scene when the movie is over or a
 * button skips it. */
void MovieTask::update(s32 arg) {
    MoviePlayer *player;
    PadManager *pads;
    u32 pressed;

    if (frames >= 0) {
        player = this->player;
        pads = PAD_MANAGER_INSTANCE;
        player->update();
        pressed = padManagerGetPressed(pads, -1);
        if (player->isFinished() || (pressed & MOVIE_SKIP_BUTTONS)) {
            frames = -1;
            taskKill(this);
        } else {
            frames++;
        }
    }
}

/* Constructs what to play: the frames of a file, in 24-bit color, on the
 * whole screen. The frames are -1 if the file is not on the disc. */
MovieParams::MovieParams(char *name, s32 startFrame, s32 endFrame) {
    rgb24 = 1;
    this->startFrame = startFrame;
    this->endFrame = endFrame;
    x = 0;
    y = 0;
    width = SCREEN_WIDTH;
    height = SCREEN_HEIGHT;
    /* findIsoFile fills a CdlFILE, as CdSearchFile does */
    if (findIsoFile((CdfsFile *)&file, name) == 0) {
        this->startFrame = -1;
        this->endFrame = -1;
    }
}

/* Sets the color depth: 24-bit, else 15-bit. */
void MovieParams::setRgb24(s32 rgb24) {
    this->rgb24 = rgb24;
}

/* Sets where the movie is on the screen. */
void MovieParams::setPosition(s32 x, s32 y) {
    this->x = x;
    this->y = y;
}

/* Sets the size of the screen. */
void MovieParams::setSize(s32 width, s32 height) {
    this->width = width;
    this->height = height;
}

/* Returns whether there are frames to play. */
s32 MovieParams::isValid() {
    return startFrame >= 0 && startFrame < endFrame;
}

/* Constructs a player of what params says, and clears the screen. */
MoviePlayer::MoviePlayer(MovieParams *source) : params(*source) {
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    MOVIE_PLAYER = this;
    params.isValid(); /* its answer is not used */
    firstSlice = 1;
    /* the two places in VRAM, one under the other. The match depends on
     * working them all out before the stores: the second x takes the
     * first's loads of x */
    x0 = params.rgb24 ? params.x * 3 / 2 : params.x;
    y0 = params.y;
    x1 = params.rgb24 ? params.x * 3 / 2 : params.x;
    y1 = params.y + params.height;
    vlcId = 0;
    imageId = 0;
    rectId = 0;
    frameWidth = 0;
    frameHeight = 0;
    decoded = 0;
    started = 0;
    streaming = 0;
    rects[0].x = x0;
    rects[0].y = y0;
    rects[1].x = x1;
    rects[1].y = y1;
    slice.w = params.rgb24 ? 24 : 16;
    unk64 = params.rgb24;
    slice.x = x0;
    slice.y = y0;
    clearScreen();
    SetDefDispEnv(&dispEnv, 0, params.height, params.width, params.height);
}

/* Destroys a player: stops the movie. */
MoviePlayer::~MoviePlayer() {
    stop();
    MOVIE_PLAYER = NULL;
}

/* Stops the movie if it plays: the XA sound, the decoding and the stream. */
void MoviePlayer::stop() {
    if (streaming) {
        streaming = 0;
        started = 0;
        setXaVolume(0);
        clearScreen();
        DecDCToutCallback(NULL);
        StUnSetRing();
        func_80046D50(CdlPause, NULL, NULL);
    }
}

/* Returns the display the frames are shown in. */
DISPENV *MoviePlayer::getDispEnv() {
    return &dispEnv;
}

/* Starts the movie if it is not playing: the stream from the file's start,
 * the XA sound, and the first frame's VLC. */
void MoviePlayer::start() {
    if (!started) {
        openStream(&params.file.pos);
        setXaVolume(XA_VOLUME_ON);
        DecDCTvlcBuild(vlcTable);
        while (decodeVlc() == -1) {
            loc = params.file.pos;
            movieReadStream(&loc);
        }
        finished = 0;
        streaming = 1;
        started = 1;
    }
}

/* Plays a frame: decodes the frame whose VLC is ready into VRAM while the
 * VLC of the next is decoded, then shows it. */
void MoviePlayer::update() {
    s32 sector;
    s32 id;

    if (isFinished()) {
        return;
    }
    start();
    DecDCTin(vlcBuffers[vlcId], params.rgb24 ? 3 : 2);
    DecDCTout(imageBuffers[imageId], slice.w * ((slice.h + 15) & ~15) / 2);
    while (decodeVlc() < 0) {
        /* the stream got lost: read it again from where it is, or from the
         * start */
        sector = StGetBackloc(&loc);
        if (sector > params.endFrame || sector <= 0) {
            loc = params.file.pos;
        }
        movieReadStream(&loc);
    }
    sync();
    afterDecode();
    /* show the place in VRAM the frame was decoded to. The match depends on
     * dispX and dispW: the game takes &disp.x and &disp.w ($s2, $s5) before
     * working them out and stores through them; written as plain fields,
     * GCC stores at their offsets from this */
    id = rectId ^ 1;
    short &dispX = dispEnv.disp.x;
    dispX = rects[id].x - (params.rgb24 ? params.x * 3 / 2 : params.x);
    dispEnv.disp.y = rects[id].y - params.y;
    short &dispW = dispEnv.disp.w;
    dispW = params.rgb24 ? params.width * 3 / 2 : params.width;
    dispEnv.disp.h = params.height;
    if (params.rgb24) {
        dispEnv.disp.w = dispEnv.disp.w * 2 / 3;
        dispEnv.isrgb24 = params.rgb24;
    }
    beforeShow(&dispEnv);
    if (isFinished()) {
        stop();
    }
}

/* Returns whether the end frame was read. */
s32 MoviePlayer::isFinished() {
    return finished;
}

/* Clears both places the frames are decoded to, with the display off. */
void MoviePlayer::clearScreen() {
    RECT rect;

    VSync(0);
    SetDispMask(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = params.rgb24 ? params.width * 3 / 2 : params.width;
    rect.h = params.height * 2;
    ClearImage(&rect, 0, 0, 0);
    VSync(0);
    SetDispMask(1);
}

/* Sets the MDEC and the stream up, and starts reading at loc. */
void MoviePlayer::openStream(CdlLOC *loc) {
    DecDCTReset(0);
    DecDCToutCallback(movieSliceDecoded);
    StSetRing(ring, MOVIE_RING_SECTORS);
    StSetStream(params.rgb24 != 0, params.startFrame, -1, NULL, NULL);
    movieReadStream(loc);
}

/* Starts streaming from loc, at double speed with the XA sound. */
void movieReadStream(CdlLOC *loc) {
    func_80045820(loc, CdlModeStream2 | CdlModeSpeed | CdlModeRT);
}

/* The MDEC's callback: a slice is decoded. */
void movieSliceDecoded(void) {
    MOVIE_PLAYER->loadSlice();
}

/* Loads the slice the MDEC decoded into VRAM and has it decode the next, or
 * ends the frame. */
void MoviePlayer::loadSlice() {
    RECT decodedSlice;
    s32 id;
    s32 rest;

    if (unk64 && D_80116914) {
        StCdInterrupt();
        D_80116914 = 0;
    }
    id = imageId;
    decodedSlice = slice;
    imageId = id ^ 1;
    if (firstSlice && (rest = rects[rectId].w % slice.w) != 0) {
        /* the first slice is what the frame has past a whole number of slices */
        slice.x += rest;
        firstSlice = 0;
    } else {
        slice.x += slice.w;
    }
    if (slice.x < rects[rectId].x + rects[rectId].w) {
        DecDCTout(imageBuffers[imageId], slice.w * ((slice.h + 15) & ~15) / 2);
    } else {
        decoded = 1;
        firstSlice = 1;
        rectId ^= 1;
        slice.x = rects[rectId].x;
        slice.y = rects[rectId].y;
    }
    func_80055B28(&decodedSlice, imageBuffers[id]);
}

/* Decodes the VLC of the next frame; -1 if none came in time. */
s32 MoviePlayer::decodeVlc() {
    s32 tries = MOVIE_FRAME_TRIES;
    u_long *frame;

    while ((frame = getNextFrame()) == NULL) {
        if (--tries == 0) {
            return -1;
        }
    }
    vlcId ^= 1;
    DecDCTvlc2(frame, vlcBuffers[vlcId], vlcTable);
    StFreeRing(frame);
    return 0;
}

/* Returns the next frame in the ring, NULL if none came in time. A frame of
 * another size clears the screen first. */
u_long *MoviePlayer::getNextFrame() {
    RECT rect;
    u_long *frame;
    StHEADER *header;
    s32 tries = MOVIE_FRAME_TRIES;

    while (StGetNext(&frame, (u_long **)&header)) {
        if (--tries == 0) {
            return NULL;
        }
    }
    finished |= header->frameCount >= (u32)params.endFrame;
    if (frameWidth != header->width || frameHeight != header->height) {
        rect.x = 0;
        rect.y = 0;
        rect.w = params.rgb24 ? params.width * 3 / 2 : params.width;
        rect.h = params.height * 2;
        ClearImage(&rect, 0, 0, 0);
        frameWidth = header->width;
        frameHeight = header->height;
    }
    /* the size of the frame in both places in VRAM. The match depends on
     * width1 and width0: the game takes &rects[1].w and &rects[0].w ($a2,
     * $a1) before working the width out and stores through them; written as
     * plain fields, GCC stores at their offsets from this */
    short &width1 = rects[1].w;
    short &width0 = rects[0].w;

    width1 = width0 = params.rgb24 ? frameWidth * 3 / 2 : frameWidth;
    slice.h = rects[1].h = rects[0].h = frameHeight;
    return frame;
}

/* Waits for the frame to be decoded; gives it up after a while. Its C
 * matches only with `decoded` volatile: the game keeps the last store out of
 * the return's delay slot. */
INCLUDE_ASM("asm/jp/movie/nonmatchings/movie", moviePlayerSync);

/* Does nothing: a player that does something after a frame is decoded
 * overrides it. */
void MoviePlayer::afterDecode() {
}

/* Does nothing: a player that changes the display a frame is shown in
 * overrides it. */
void MoviePlayer::beforeShow(DISPENV *dispEnv) {
}
