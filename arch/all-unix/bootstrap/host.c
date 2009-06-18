#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "host.h"

#define D(x)

void *Host_HostLib_Open (const char *filename, char **error) {
    void *handle;

    D(bug("[hostlib] Open: filename=%s\n", filename));
    
    handle = dlopen((char *) filename, RTLD_NOW);

    if (error != NULL)
        *error = handle == NULL ? dlerror() : NULL;

    return handle;
}

int Host_HostLib_Close (void *handle, char **error) {
    int ret;

    D(printf("[hostlib] Open: handle=0x%08x\n", handle));

    ret = dlclose(handle);

    if (error != NULL)
        *error = ret != 0 ? dlerror() : NULL;

    return ret;
}

void Host_HostLib_FreeErrorStr (char *error) {
    /* libdl returns static strings, so nothing to do here */
}

void *Host_HostLib_GetPointer (void *handle, const char *symbol, char **error) {
    void *ptr;

    D(printf("[hostlib] GetPointer: handle=0x%08x, symbol=%s\n", handle, symbol));

    dlerror();

    ptr = dlsym(handle, (char *) symbol);

    if (error != NULL)
        *error = dlerror();

    return ptr;
}

int Host_HostLib_GetInterface (void *handle, char **names, void **funcs) {
    int unresolved = 0;
    char *err;

    dlerror();
    for (; *names != NULL; names++, funcs++) {
        *funcs = dlsym(handle, *names);
        D(printf("[hostlib] GetInterface: handle=0x%08x, symbol=%s, value=0x%08x\n", handle, *names, *funcs));
        if (*funcs == NULL && (err = dlerror()) != NULL) {
            D(printf("[hostlib] error fetching value for symbol '%s': %s\n", *names, err));
            unresolved++;
        }
    }

    return unresolved;
}

int Host_VKPrintF (const char *fmt, va_list args) {
    return vprintf(fmt, args);
}

int Host_PutChar (int c) {
    return putchar(c);
}

extern char bootstrap_dir[];
extern char *bootstrap_bin;
extern char **bootstrap_args;

void Host_Shutdown (int action) {
    switch (action) {
        case 0:
            printf("[shutdown] power off\n");
            exit(0);

        case 1:
            printf("[shutdown] cold reboot\n");
            chdir(bootstrap_dir);
            execvp(bootstrap_bin, bootstrap_args);
            D(printf("[shutdown] reboot failed: %s\n", strerror(errno)));
    }
}
