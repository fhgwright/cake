#define DEBUG 0

#include <inttypes.h>
#include <aros/symbolsets.h>
#include <exec/lists.h>
#include <memory.h>
#include <exec/resident.h>
#include <exec/memheaderext.h>
#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/alib.h>
#include <proto/kernel.h>

#include <aros/kernel.h>
#include <aros/debug.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "kernel_intern.h"
#include LC_LIBDEFS_FILE

#define HOST_MODULE "Libs/Host/libkernel.so"

extern struct ExecBase * PrepareExecBase(struct MemHeader *);
extern ULONG ** Exec_RomTagScanner(struct ExecBase*,UWORD**);

static struct TagItem *BootMsg;
struct HostInterface *HostIFace;
struct KernelInterface KernelIFace;
APTR KernelBase = NULL;
/* static char cmdLine[200]; TODO */

#undef kprintf
#undef rkprintf
#undef vkprintf

int mykprintf(const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    if (SysBase)
        Forbid();
    r = HostIFace->VKPrintF(fmt, args);
    if (SysBase)
        Permit();
    va_end(args);
    return r;
}

int myvkprintf (const char *fmt, va_list args)
{
    int res;
    
    if (SysBase)
        Forbid();
    res = HostIFace->VKPrintF(fmt, args);
    if (SysBase)
        Permit();
    return res;
}

int myrkprintf(const char *foo, const char *bar, int baz, const char *fmt, ...)
{
  va_list args;
  int r;

  va_start(args, fmt);
  if (SysBase)
      Forbid();
  r = HostIFace->VKPrintF(fmt, args);
  if (SysBase)
      Permit();
  va_end(args);
  return r;
}

AROS_LH0I(struct TagItem *, KrnGetBootInfo,
         struct KernelBase *, KernelBase, 1, Kernel)
{
    AROS_LIBFUNC_INIT

    return BootMsg;
    
    AROS_LIBFUNC_EXIT
}

/* auto init */
static int Kernel_Init(LIBBASETYPEPTR kBase)
{
  int i;

  KernelBase = kBase;
  D(mykprintf("[Kernel] init (KernelBase=%p)\n", kBase));
  D(mykprintf("[Kernel] -1 : %p -2 : %p\n", *((APTR*)(((APTR*)kBase)-1)),*((APTR*)(((APTR*)kBase)-2))));
  for (i=0; i < EXCEPTIONS_NUM; i++)
        NEWLIST(&kBase->kb_Exceptions[i]);

  for (i=0; i < INTERRUPTS_NUM; i++)
        NEWLIST(&kBase->kb_Interrupts[i]);
  D(mykprintf("[Kernel] KrnGetBootInfo yields %p\n",Kernel_KrnGetBootInfo()));
  return 1;
}

ADD2INITLIB(Kernel_Init, 0)

char *kernel_functions[] = {
    "core_init",
    "core_intr_disable",
    "core_intr_enable",
    "core_is_super",
    "core_syscall",
    "core_get_context_size",
    "core_print_context",
    "core_prepare_context",
    NULL
};

/* rom startup */


//make this the entry point
//int startup(struct TagItem *msg) __attribute__ ((section (".aros.init")));

int __startup startup(struct TagItem *msg) {
    void *libkernel;
    char *err;
    int unresolved;
    struct MemHeader *mh;

    BootMsg = msg;

    HostIFace = (struct HostInterface *) krnGetTagData(KRN_HostInterface, 0, msg);

    mykprintf("[kernel] starting up\n");

    mykprintf("[kernel] loading host module '%s'\n", HOST_MODULE);

    libkernel = HostIFace->HostLib_Open(HOST_MODULE, &err);
    if (libkernel == NULL) {
        mykprintf("[kernel] failed to load host module: %s\n", err);
        HostIFace->HostLib_FreeErrorStr(err);
        return -1;
    }

    unresolved = HostIFace->HostLib_GetInterface(libkernel, kernel_functions, (void **) &KernelIFace);
    if (unresolved > 0) {
        char **names;
        void **funcs;

        mykprintf("[kernel] missing symbols in host module:\n");

        for (names = kernel_functions, funcs = (void **) &KernelIFace; *names != NULL; names++, funcs++) {
            if (*funcs == NULL)
                mykprintf("             %s\n", *names);
        }

        HostIFace->HostLib_Close(libkernel, NULL);

        return -1;
    }

    void *kernel = (void *) krnGetTagData(KRN_KernelLowest, 0, msg);
    void *kernel_end = (void *) krnGetTagData(KRN_KernelHighest, 0, msg);
    void *memory = (void *) krnGetTagData(KRN_MEMLower, 0, msg);
    void *memory_end = (void *) krnGetTagData(KRN_MEMUpper, 0, msg);

    mykprintf("[kernel] system memory at 0x%x-0x%x (0x%x bytes)\n", memory, memory_end, memory_end-memory+1);
    mykprintf("[kernel] kernel memory at 0x%x-0x%x (0x%x bytes)\n", kernel, kernel_end, kernel_end-kernel+1);

    /* Prepare the first mem header and hand it to PrepareExecBase to take SysBase live */
    D(mykprintf("[kernel] preparing system memory header\n"));

    mh = memory;
    mh->mh_Node.ln_Type  = NT_MEMORY;
    mh->mh_Node.ln_Name = "chip memory";
    mh->mh_Node.ln_Pri = -5;
    mh->mh_Attributes = MEMF_CHIP | MEMF_PUBLIC | MEMF_LOCAL | MEMF_24BITDMA | MEMF_KICK;
    mh->mh_First = memory + MEMHEADER_TOTAL;
    mh->mh_First->mc_Next = NULL;
    mh->mh_First->mc_Bytes = memory_end + 1 - memory - MEMHEADER_TOTAL;
    mh->mh_Lower = memory;
    mh->mh_Upper = memory_end;
    mh->mh_Free = mh->mh_First->mc_Bytes;

    /*
     * FIXME: This routine is part of exec.library, however it doesn't have an LVO
     * (it can't have one because exec.library is not initialized yet) and is called
     * only from here. Probably the code should be reorganized
     */
    D(mykprintf("[kernel] initialising SysBase\n"));
    SysBase = PrepareExecBase(mh);
    mykprintf("[kernel] SysBase initialised at 0x%x\n", SysBase);
  
    /* ROM memory header. This special memory header covers all ROM code and data sections
     * so that TypeOfMem() will not return 0 for addresses pointing into the kernel.
     */
    D(mykprintf("[kernel] preparing kernel memory header\n"));
    if ((mh = (struct MemHeader *) AllocMem(sizeof(struct MemHeader), MEMF_PUBLIC))) {
        mh->mh_Node.ln_Type = NT_MEMORY;
        mh->mh_Node.ln_Name = "rom memory";
        mh->mh_Node.ln_Pri = -128;
        mh->mh_Attributes = MEMF_KICK;
        mh->mh_First = NULL;
        mh->mh_Lower = kernel;
        mh->mh_Upper = kernel_end;
        mh->mh_Free = 0;                            /* Never allocate from this chunk! */
        Enqueue(&SysBase->MemList, &mh->mh_Node);
    }

    D(mykprintf("[kernel] initialising debug print functions\n"));

    struct AROSSupportBase *debug = (struct AROSSupportBase *) SysBase->DebugAROSBase;
    debug->kprintf = mykprintf;
    debug->rkprintf = myrkprintf;
    debug->vkprintf = myvkprintf;

    /*
     * FIXME: Cross-module call again
     */
    mykprintf("[kernel] scanning modules\n");
    UWORD *ranges[] = { kernel, kernel_end, (UWORD *) -1 };
    SysBase->ResModules = Exec_RomTagScanner(SysBase, ranges);

    mykprintf("[kernel] initialising host module\n");
    if (KernelIFace.core_init(SysBase->VBlankFrequency, &SysBase, &KernelBase) < 0) {
        mykprintf("[kernel] failed to initialise host module\n");
        return -1;
    }

  mykprintf("[Kernel] calling InitCode(RTF_SINGLETASK,0)\n");
  InitCode(RTF_SINGLETASK, 0);

  mykprintf("leaving startup!\n");
  HostIFace->HostLib_Close(libkernel, NULL);
  return 1;
}


struct TagItem *krnNextTagItem(const struct TagItem **tagListPtr)
{
    if (!(*tagListPtr)) return 0;

    while(1)
    {
        switch((*tagListPtr)->ti_Tag)
        {
            case TAG_MORE:
                if (!((*tagListPtr) = (struct TagItem *)(*tagListPtr)->ti_Data))
                    return NULL;
                continue;
            case TAG_IGNORE:
                break;

            case TAG_END:
                (*tagListPtr) = 0;
                return NULL;

            case TAG_SKIP:
                (*tagListPtr) += (*tagListPtr)->ti_Data + 1;
                continue;

            default:
                return (struct TagItem *)(*tagListPtr)++;

        }

        (*tagListPtr)++;
    }
}

struct TagItem *krnFindTagItem(Tag tagValue, const struct TagItem *tagList)
{
    struct TagItem *tag;
    const struct TagItem *tagptr = tagList;

    while((tag = krnNextTagItem(&tagptr)))
    {
        if (tag->ti_Tag == tagValue)
            return tag;
    }

    return 0;
}

IPTR krnGetTagData(Tag tagValue, intptr_t defaultVal, const struct TagItem *tagList)
{
    struct TagItem *ti = 0;

    if (tagList && (ti = krnFindTagItem(tagValue, tagList)))
        return ti->ti_Data;

        return defaultVal;
}

