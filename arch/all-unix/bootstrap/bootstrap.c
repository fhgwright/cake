#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <aros/system.h>

#include <aros/kernel.h>
#include <utility/tagitem.h>

#include "debug.h"
#include "elfloader32.h"
#include "hostlib.h"
#include "shutdown.h"
#include "../kernel/hostinterface.h"

#define D(x) x

#define DEFAULT_KERNEL  "boot/aros-unix"
#define DEFAULT_MEMSIZE (64)

static unsigned char __bss_track[32768];
struct TagItem km[64];
char bootstrapdir[PATH_MAX];

char *bootstrap_bin;
char **bootstrap_args;
char *kernel_bin = DEFAULT_KERNEL;

char kernel_args[256];

typedef int (*kernel_entry_fun_t)(struct TagItem *);

/*
 * Some helpful functions that link us to the underlying host OS.
 * Without them we would not be able to estabilish any interaction with it.
 */
struct HostInterface HostIFace = {
    Host_HostLib_Open,
    Host_HostLib_Close,
    Host_HostLib_GetPointer,
    Host_HostLib_FreeErrorStr,
    Host_HostLib_GetInterface,
    Host_VKPrintF,
    Host_PutChar,
    Host_Shutdown
};


static void usage (void) {
    printf ("usage: %s [options] [--] [kernel arguments]\n"
	    "availible options:\n"
            " -h         show this help\n"
            " -m <size>  allocate <size> megabytes of memory for AROS\n"
            "                (default is %d)\n"
            " -k <file>  use <file> as a kernel\n"
            "                (default is %s)\n",
            bootstrap_bin, DEFAULT_MEMSIZE, DEFAULT_KERNEL);
}

int main (int argc, char **argv) {
    struct utsname utsname;
    char host_version[256];
    char opt;
    uint32_t memsize = DEFAULT_MEMSIZE << 20;
    char *c;
    int i;
    struct stat st;
    FILE *kernel;

    printf("AROS for Linux, built " __DATE__ "\n");

    getcwd(bootstrapdir, PATH_MAX);
    bootstrap_bin = argv[0];
    bootstrap_args = argv;

    while ((opt = getopt(argc, argv, "m:k:h?")) >= 0) {
        switch (opt) {
            case 'm':
                memsize = atoi(optarg) << 20;
                continue;
            
            case 'k':
                kernel_bin = optarg;
                continue;

            default:
                usage();
                return 1;
        }
    }

    uname(&utsname);
    snprintf(host_version, sizeof(host_version), "%s %s %s %s %s", utsname.sysname, utsname.nodename, utsname.release, utsname.version, utsname.machine);
    printf("[boot] OS version: %s\n", host_version);
    
    c = kernel_args;
    for (i = optind; i < argc; i++) {
        strcpy(c, argv[i]);
        c += strlen(argv[i]);
        *c++ = ' ';
    }
    *--c = '\0';

    printf("[boot] kernel arguments: %s\n", kernel_args);

    if (stat("../AROS.boot", &st) == 0) {
        chdir("..");
    }

    void *memory = mmap(NULL, memsize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!memory) {
            fprintf(stderr, "[boot] failed to allocate memory for system\n");
            return -1;
    }
    printf("[boot] allocated 0x%x bytes at 0x%x for system memory\n", memsize, memory);
    
    kernel = fopen(kernel_bin, "rb");
    if (kernel == NULL) {
        fprintf(stderr, "[boot] unable to open kernel '%s': %s\n", kernel_bin, strerror(errno));
        return -1;
    }

    set_base_address(__bss_track);
    if (load_elf_file(kernel, memory) != 0) {
        fclose(kernel);
        fprintf(stderr, "[boot] failed to load kernel '%s'\n", kernel_bin);
        return -1;
    }
    fclose(kernel);

    kernel_entry_fun_t kernel_entry_fun = kernel_entry();

    //fill in kernel message
    struct TagItem *tag = km;

    tag->ti_Tag = KRN_MEMLower;
    tag->ti_Data = (unsigned long)memory;
    tag++;
    
    tag->ti_Tag = KRN_MEMUpper;
    tag->ti_Data = memory + memsize - 1;
    tag++;

    tag->ti_Tag = KRN_KernelLowest;
    tag->ti_Data = kernel_entry_fun;
    tag++;
        
    tag->ti_Tag = KRN_KernelHighest;
    tag->ti_Data = kernel_highest();
    tag++;

    tag->ti_Tag = KRN_KernelBss;
    tag->ti_Data = (unsigned long)__bss_track;
    tag++;

    tag->ti_Tag = KRN_BootLoader;
    tag->ti_Data = host_version;
    tag++;

    tag->ti_Tag = KRN_CmdLine;
    tag->ti_Data = kernel_args;
    tag++;
    
    tag->ti_Tag = KRN_HostInterface;
    tag->ti_Data = &HostIFace;
    tag++;

    tag->ti_Tag = TAG_DONE;

    printf("[Bootstrap] entering kernel@%p...\n",kernel_entry_fun);
    int retval = kernel_entry_fun(km);

    printf("kernel returned %i\n",retval);
}    
