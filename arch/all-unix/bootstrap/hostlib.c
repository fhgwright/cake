#include <stdio.h>
#include <dlfcn.h>


#include "hostlib.h"

#define D(x)

void Host_HostLib_FreeErrorStr(char *error)
{
    /* libdl returns static strings, so nothing to do here */
}

void *Host_HostLib_Open(const char *filename, char **error)
{
    void *handle;

    D(bug("[hostlib] Open: filename=%s\n", filename));
    
    handle = dlopen((char *) filename, RTLD_NOW);

    if (error != NULL)
        *error = handle == NULL ? dlerror() : NULL;

    return handle;
}

int Host_HostLib_Close(void *handle, char **error)
{
    int ret;

    D(bug("[hostlib] Close: handle=0x%08x\n", handle));

    ret = dlclose(handle);

    if (error != NULL)
        *error = ret != 0 ? dlerror() : NULL;

    return ret;
}

void *Host_HostLib_GetPointer(void *handle, const char *symbol, char **error)
{
    void *ptr;

    D(bug("[hostlib] GetPointer: handle=0x%08x, symbol=%s\n", handle, symbol));

    dlerror();

    ptr = dlsym(handle, (char *) symbol);

    if (error != NULL)
        *error = dlerror();

    return ptr;
}

unsigned long Host_HostLib_GetInterface(void *handle, char **names, void **funcs)
{
    unsigned long unresolved = 0;

    for (; *names; names++) {
        *funcs = dlsym(handle, *names);
        D(printf("[hostlib] GetInterface: handle=0x%08x, symbol=%s, value=0x%08x\n", handle, *names, *funcs));
        if (*funcs++ == NULL)
            unresolved++;
    }
    return unresolved;
}
