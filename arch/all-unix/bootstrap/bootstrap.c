#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include <aros/system.h>

#include <aros/kernel.h>
#include <utility/tagitem.h>

#include "debug.h"
#include "elfloader32.h"
#include "hostlib.h"
#include "shutdown.h"
#include "../kernel/hostinterface.h"

#define D(x)

#define DEFAULT_KERNEL  "boot/aros-unix"
#define DEFAULT_MEMSIZE (64)

static unsigned char __bss_track[32768];
struct TagItem km[64];
char bootstrapdir[PATH_MAX];
char SystemVersion[256];

char *bootstrap_bin;
char **bootstrap_args;
char *kernel_bin = DEFAULT_KERNEL;

char cmdline[256];

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

void *SysBase;

static void usage (void) {
    printf ("AROS for Linux\n"
            "usage: %s [options] [--] [kernel arguments]\n"
	    "Availible options:\n"
            " -h                 show this page\n"
            " -m <size>          allocate <size> Megabytes of memory for AROS\n"
            "                    (default is %d)\n"
            " -k <file>          use <file> as a kernel\n"
            "                    (default is %s)\n"
            "\n"
            "Please report bugs to the AROS development team. http://www.aros.org/\n",
            bootstrap_bin, DEFAULT_MEMSIZE, DEFAULT_KERNEL);
}

int main (int argc, char **argv) {
    char *error;
    unsigned long BadSyms;
    struct TagItem *t;
    int x;
    struct stat st;
    int i;
    unsigned int memSize = DEFAULT_MEMSIZE;
    char *KernelArgs = NULL;
    struct utsname utsname;
    char opt;

    getcwd(bootstrapdir, PATH_MAX);
    bootstrap_bin = argv[0];
    bootstrap_args = argv;

    while ((opt = getopt(argc, argv, "m:k:h?")) >= 0) {
        switch (opt) {
            case 'm':
                memSize = atoi(optarg);
                continue;
            
            case 'k':
                kernel_bin = optarg;
                continue;

            default:
                usage();
                return 1;
        }
    }

    D(printf("[Bootstrap] %ld arguments processed\n", optind));

    KernelArgs = cmdline;
    for (i = optind; i < argc; i++) {
        strcpy(KernelArgs, argv[i]);
        KernelArgs += strlen(argv[i]);
        *KernelArgs++ = ' ';
    }
    KernelArgs--;
    *KernelArgs = '\0';

    D(printf("[Bootstrap] Kernel arguments: %s\n", KernelArgs));

    uname(&utsname);
    sprintf(SystemVersion, "%s %s %s %s %s", utsname.sysname, utsname.nodename, utsname.release, utsname.version, utsname.machine);
    D(printf("[Bootstrap] OS version: %s\n", SystemVersion));
    
    if (!stat("..\\AROS.boot", &st)) {
            chdir("..");
    }

    //load elf-kernel and fill in the bootinfo
    void * file = fopen(kernel_bin, "rb");

    if (!file)
    {
    	printf("[Bootstrap] unable to open kernel \"%s\"\n", kernel_bin);
    	return -1;
    }
    set_base_address(__bss_track, &SysBase);
    i = load_elf_file(file,0);
    fclose(file);
    if (!i) {
            printf("[Bootstrap] Failed to load kernel \"%s\"\n", kernel_bin);
            return -1;
    }
    D(printf("[Bootstrap] allocating working mem: %iMb\n",memSize));

    size_t memlen = memSize << 20;
    void * memory = malloc(memlen);

    if (!memory) {
            printf("[Bootstrap] Failed to allocate RAM!\n");
            return -1;
    }
    D(printf("[Bootstrap] RAM memory allocated: %p-%p (%lu bytes)\n", memory, memory + memlen, memlen));
    
    kernel_entry_fun_t kernel_entry_fun = kernel_entry();

    //fill in kernel message
    struct TagItem *tag = km;

    tag->ti_Tag = KRN_MEMLower;
    tag->ti_Data = (unsigned long)memory;
    tag++;
    
    tag->ti_Tag = KRN_MEMUpper;
    tag->ti_Data = memory + memlen - 1;
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
    tag->ti_Data = SystemVersion;
    tag++;

    tag->ti_Tag = KRN_CmdLine;
    tag->ti_Data = cmdline;
    tag++;
    
    tag->ti_Tag = KRN_HostInterface;
    tag->ti_Data = &HostIFace;
    tag++;

    tag->ti_Tag = TAG_DONE;

    printf("[Bootstrap] entering kernel@%p...\n",kernel_entry_fun);
    int retval = kernel_entry_fun(km);

    printf("kernel returned %i\n",retval);
}    
