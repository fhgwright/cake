#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "shutdown.h"

#define D(x)

void Host_Shutdown(unsigned long action)
{
    switch (action) {
    case SD_ACTION_POWEROFF:
        D(printf("[Shutdown] POWER OFF request\n"));
        exit(0);
    	break;
    case SD_ACTION_COLDREBOOT:
        D(printf("[Shutdown] Cold reboot, dir: %s, name: %s, command line: %s\n", bootstrapdir, bootstrapname, cmdline));
        chdir(bootstrapdir);
        execvp(bootstrapname, cmdline);
        D(printf("[Shutdown] Unable to re-run AROS: %s\n", strerror(errno)));
    }
}
