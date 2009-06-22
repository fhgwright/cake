#ifndef HOST_H
#define HOST_H

#include <stdarg.h>

void *Host_HostLib_Open (const char *filename, char **error);
int   Host_HostLib_Close (void *handle, char **error);
void  Host_HostLib_FreeErrorStr (char *error);
void *Host_HostLib_GetPointer (void *handle, const char *symbol, char **error);
int   Host_HostLib_GetInterface (void *handle, char **names, void **funcs);
int   Host_VKPrintF (const char *fmt, va_list args);
int   Host_PutChar (int c);
void  Host_Shutdown (int action);

#endif
