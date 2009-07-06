#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <aros/system.h>

#include <aros/kernel.h>
#include <utility/tagitem.h>

#include "elfloader32.h"
#include "host.h"

#include "../kernel/hostinterface.h"

#define D(x) x

#define DEFAULT_KERNEL  "boot/aros-unix"
#define DEFAULT_MEMSIZE (64)

struct TagItem kernel_tags[8];

char bootstrap_dir[PATH_MAX];
char *bootstrap_bin;
char **bootstrap_args;
char *kernel_bin = DEFAULT_KERNEL;

char kernel_args[256];

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
    char host_version[256], opt, *c;
    uint32_t memsize = DEFAULT_MEMSIZE << 20, imagesize, kernsize;
    int i, fd;
    struct stat st;
    void *kernel, *memory, *image, *start, *end, *entry;

    printf("AROS for Linux, built " __DATE__ "\n");

    getcwd(bootstrap_dir, PATH_MAX);
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

    fd = open(kernel_bin, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[boot] unable to open kernel '%s': %s\n", kernel_bin, strerror(errno));
        return -1;
    }

    imagesize = lseek(fd, 0, SEEK_END);
    image = mmap(NULL, imagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (image == MAP_FAILED) {
        fprintf(stderr, "[boot] unable to map kernel image: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    kernsize = elf_count_allocation(image);
    if (kernsize == -1) {
        fprintf(stderr, "[boot] couldn't determine size of kernel memory\n");
        munmap(image, imagesize);
        close(fd);
        return -1;
    }

    kernel = mmap(NULL, kernsize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (kernel == MAP_FAILED) {
        fprintf(stderr, "[boot] failed to map 0x%x bytes for kernel memory: %s\n", kernsize, strerror(errno));
        munmap(image, imagesize);
        close(fd);
        return -1;
    }

    printf("[boot] allocated 0x%x bytes at 0x%x for kernel memory\n", kernsize, kernel);

    memory = mmap(NULL, memsize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        fprintf(stderr, "[boot] failed to allocate 0x%x bytes for system memory: %s\n", memsize, strerror(errno));
        munmap(kernel, kernsize);
        munmap(image, imagesize);
        close(fd);
        return -1;
    }

    printf("[boot] allocated 0x%x bytes at 0x%x for system memory\n", memsize, memory);
    
    if (elf_load_image(image, kernel, 0, &start, &end, &entry) != 0) {
        fprintf(stderr, "[boot] failed to load kernel '%s'\n", kernel_bin);
        munmap(memory, memsize);
        munmap(kernel, kernsize);
        munmap(image, imagesize);
        close(fd);
        return -1;
    }

    munmap(image, imagesize);
    close(fd);

    printf("[boot] kernel image '%s' loaded\n", kernel_bin);

    kernel_tags[0].ti_Tag = KRN_MEMLower;
    kernel_tags[0].ti_Data = (STACKIPTR) memory;
    
    kernel_tags[1].ti_Tag = KRN_MEMUpper;
    kernel_tags[1].ti_Data = (STACKIPTR) memory + memsize - 1;

    kernel_tags[2].ti_Tag = KRN_KernelLowest;
    kernel_tags[2].ti_Data = (STACKIPTR) start;
        
    kernel_tags[3].ti_Tag = KRN_KernelHighest;
    kernel_tags[3].ti_Data = (STACKIPTR) end;

    kernel_tags[4].ti_Tag = KRN_BootLoader;
    kernel_tags[4].ti_Data = (STACKIPTR) host_version;

    kernel_tags[5].ti_Tag = KRN_CmdLine;
    kernel_tags[5].ti_Data = (STACKIPTR) kernel_args;
    
    kernel_tags[6].ti_Tag = KRN_HostInterface;
    kernel_tags[6].ti_Data = (STACKIPTR) &HostIFace;

    kernel_tags[7].ti_Tag = TAG_DONE;

    printf("[boot] handing control to kernel\n");

    int retval = ((int (*)(struct TagItem *)) entry)(kernel_tags);

    printf("[boot] kernel returned %d\n", retval);

    munmap(memory, memsize);
    munmap(kernel, kernsize);

    return 0;
}    