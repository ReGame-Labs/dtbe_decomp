#include "common.h"
#include "engine/system/load_exec.h"
#include "engine/lib/string.h"
#include "libapi.h"
#include "libpad.h"
#include "stdio.h"
#include "psyq.h"

/* the top of the stack of the executable run by runExecutable */
#define EXEC_STACK_TOP 0x801FFFF0
/* the length of the device name "cdrom:", which the path follows */
#define EXEC_DEVICE_LENGTH (sizeof("cdrom:") - 1)

/* Runs the executable at path on the CD-ROM ("cdrom:PATH;1", with '\\' as
 * separators and in upper case), after stopping the GPU, the pads and the
 * callbacks. */
void runExecutable(char *path) {
    char file[128];

    sprintf(file, "cdrom:%s;1", path);
    copyBackslashPath(file + EXEC_DEVICE_LENGTH, file + EXEC_DEVICE_LENGTH);
    copyUpperCase(file + EXEC_DEVICE_LENGTH, file + EXEC_DEVICE_LENGTH);
    printf("LoadExec(\"%s\")\n", file);
    ResetGraph(3);
    PadStopCom();
    func_8003F874();
    _96_init();
    LoadExec(file, EXEC_STACK_TOP, 0);
}
