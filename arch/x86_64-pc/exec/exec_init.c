#include <inttypes.h>
#include <utility/tagitem.h>
#include <exec/resident.h>
#include <exec/nodes.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/bptr.h>

#include <aros/arossupportbase.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#include <aros/debug.h>
#include <aros/kernel.h>

#include <asm/cpu.h>
#include <asm/segments.h>

#include <proto/exec.h>

#include <string.h>
#include <stdio.h>

#include "../bootstrap/multiboot.h"
#include "core.h"

#define __text      __attribute__((section(".text")))
#define __no_ret    __attribute__((noreturn))
#define __packed    __attribute__((packed))

extern struct Library * PrepareAROSSupportBase (void);
extern const APTR LIBFUNCTABLE[] __text;
extern const APTR Exec_FuncTable[] __text;
void exec_DefaultTaskExit();
extern ULONG Exec_MakeFunctions(APTR, APTR, APTR, APTR);
IPTR **exec_RomTagScanner(struct TagItem *msg);
int exec_main(struct TagItem *msg, void *entry);

struct TagItem *krnNextTagItem(const struct TagItem **tagListPtr);
struct TagItem *krnFindTagItem(Tag tagValue, const struct TagItem *tagList);
IPTR krnGetTagData(Tag tagValue, intptr_t defaultVal, const struct TagItem *tagList);

/*
 * First, we will define exec.library (global) to make it usable outside this
 * file.
 */
const char exec_name[] = "exec.library";

/* Now ID string as it will be used in a minute in resident structure. */
const char exec_idstring[] = "$VER: exec 41.11 (16.12.2000)\r\n";

/* We would need also version and revision fields placed somewhere here. */
const short exec_Version = 41;
const short exec_Revision = 11;

const struct __text Resident Exec_resident =
{
        RTC_MATCHWORD,          /* Magic value used to find resident */
        &Exec_resident,         /* Points to Resident itself */
        &Exec_resident+1,       /* Where could we find next Resident? */
        0,                      /* There are no flags!! */
        41,                     /* Version */
        NT_LIBRARY,             /* Type */
        126,                    /* Very high startup priority. */
        (STRPTR)exec_name,      /* Pointer to name string */
        (STRPTR)exec_idstring,  /* Ditto */
        exec_main               /* Library initializer (for exec this value is irrelevant since we've jumped there at the begining to bring the system up */
};

void scr_RawPutChars(char *, int);
void clr();
char tab[512];
#ifdef rkprintf
#undef rkprintf
#endif
#define rkprintf(x...) scr_RawPutChars(tab, snprintf(tab, 510, x))

void _aros_not_implemented(char *string) {}
void Exec_Permit_Supervisor() {}

const char exec_chipname[] = "Chip Memory";
const char exec_fastname[] = "Fast Memory";

void __clear_bss(struct TagItem *msg)
{
    struct KernelBSS *bss;
    bss = krnGetTagData(KRN_KernelBss, 0, msg);
    
    if (bss)
    {
        while (bss->addr)
        {
            bzero(bss->addr, bss->len);
            bss++;
        }   
    }
}

static struct int_gate_64bit IGATES[256] __attribute__((used,aligned(256)));
static struct tss_64bit TSS __attribute__((used,aligned(128)));
static struct {
    struct segment_desc seg0;      /* seg 0x00 */
    struct segment_desc super_cs;  /* seg 0x08 */
    struct segment_desc super_ds;  /* seg 0x10 */
    struct segment_desc user_cs32; /* seg 0x18 */
    struct segment_desc user_ds;   /* seg 0x20 */
    struct segment_desc user_cs;   /* seg 0x28 */
    struct segment_desc tss_low;   /* seg 0x30 */
    struct segment_ext  tss_high;
} GDT __attribute__((used,aligned(128)));
/*
    The MMU pages and directories. They are stored at fixed location and may be either reused in the 
    64-bit kernel, or replaced by it. Four PDE directories (PDE2M structures) are enough to map whole
    4GB address space.
*/
static struct PML4E PML4[512] __attribute__((used,aligned(4096)));
static struct PDPE PDP[512] __attribute__((used,aligned(4096)));
static struct PDE2M PDE[4][512] __attribute__((used,aligned(4096)));

void exec_InsertMemory(struct TagItem *msg, uintptr_t lower, uintptr_t upper)
{
    struct ExecBase *SysBase = *(struct ExecBase **)4UL;
    
    uintptr_t kernLow = krnGetTagData(KRN_KernelLowest, 0, msg);
    uintptr_t kernHigh = krnGetTagData(KRN_KernelHighest, 0, msg);
    
    /* Scenario 1: Kernel area outside the affected area. */
    if (kernHigh < lower || kernLow > upper)
    {
        rkprintf("  Adding %012p - %012p\n", lower, upper);
    
        if (lower < 0x01000000)
            AddMemList(upper-lower+1,
                       MEMF_CHIP | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL | MEMF_24BITDMA,
                       -10,
                       (APTR)lower,
                       (STRPTR)exec_chipname);
        else
            AddMemList(upper-lower+1,
                       MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL,
                       0,
                       (APTR)lower,
                       (STRPTR)exec_fastname);

    }
    /* Scenario 2: Kernel area completely inside the memory region */
    else if (kernLow >= (lower+sizeof(struct MemHeader)+sizeof(struct MemChunk)) && kernHigh <= upper)
    {
        rkprintf("  Adding %012p - %012p\n", lower, upper);
    
        if (lower < 0x01000000)
            AddMemList(upper-lower+1,
                       MEMF_CHIP | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL | MEMF_24BITDMA,
                       -10,
                       (APTR)lower,
                       (STRPTR)exec_chipname);
        else
            AddMemList(upper-lower+1,
                       MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL,
                       0,
                       (APTR)lower,
                       (STRPTR)exec_fastname);
        rkprintf("  rejecting %012p - %012p\n", kernLow, kernHigh);
        AllocAbs(kernHigh-kernLow+1, kernLow);
    }
    /* Scenario 3: Kernel in lower portion of memory region */
    else if (kernLow <= (lower+sizeof(struct MemHeader)+sizeof(struct MemChunk)) && kernHigh <= upper)
    {
        lower = (kernHigh + 4095) & ~4095;
        rkprintf("  Adding %012p - %012p\n", lower, upper);
            
                if (lower < 0x01000000)
                    AddMemList(upper-lower+1,
                               MEMF_CHIP | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL | MEMF_24BITDMA,
                               -10,
                               (APTR)lower,
                               (STRPTR)exec_chipname);
                else
                    AddMemList(upper-lower+1,
                               MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL,
                               0,
                               (APTR)lower,
                               (STRPTR)exec_fastname);
    }
    /* Scenario 3: Kernel in upper portion of memory region */
    else if (kernLow >= (lower+sizeof(struct MemHeader)+sizeof(struct MemChunk)) && kernHigh >= upper)
    {
        upper = kernLow & ~4095;
        rkprintf("  Adding %012p - %012p\n", lower, upper);
            
                if (lower < 0x01000000)
                    AddMemList(upper-lower+1,
                               MEMF_CHIP | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL | MEMF_24BITDMA,
                               -10,
                               (APTR)lower,
                               (STRPTR)exec_chipname);
                else
                    AddMemList(upper-lower+1,
                               MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL,
                               0,
                               (APTR)lower,
                               (STRPTR)exec_fastname);
    }
}

int exec_main(struct TagItem *msg, void *entry)
{
    struct ExecBase *SysBase;
    int i;

    clr();
    rkprintf("AROS64 - The AROS Research OS, 64-bit version\nCompiled %s\n\n",__DATE__);

    /* Set TSS, GDT, LDT and MMU up */
    core_SetupGDT();
    core_SetupIDT();
    
    asm("int $0x20");
    
    /* Setu the 8259 up */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x11),"i"(0x20)); /* Initialization sequence for 8259A-1 */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x11),"i"(0xa0)); /* Initialization sequence for 8259A-2 */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x20),"i"(0x21)); /* IRQs at 0x20 - 0x27 */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x28),"i"(0xa1)); /* IRQs at 0x28 - 0x2f */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x04),"i"(0x21)); /* 8259A-1 is master */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x02),"i"(0xa1)); /* 8259A-2 is slave */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x01),"i"(0x21)); /* 8086 mode for both */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x01),"i"(0xa1));
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0xff),"i"(0x21)); /* Enable cascade int */
    asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0xff),"i"(0xa1)); /* Mask all interrupts */

    rkprintf("Interrupts redirected\n");

    /* Prepare the exec base */

    ULONG   negsize = LIB_VECTSIZE;             /* size of vector table */
    void  **fp      = Exec_FuncTable; //LIBFUNCTABLE;  /* pointer to a function in the table */

    rkprintf("Preparing the ExecBase...\n");

    /* Calculate the size of the vector table */
    while (*fp++ != (APTR) -1) negsize += LIB_VECTSIZE;
    SysBase = (struct ExecBase *)(0x1000 + negsize);

    rkprintf("Clearing ExecBase\n");

    /* How about clearing most of ExecBase structure? */
    bzero(&SysBase->IntVects[0], sizeof(struct ExecBase) - offsetof(struct ExecBase, IntVects[0]));

    SysBase->KickMemPtr = NULL;
    SysBase->KickTagPtr = NULL;
    SysBase->KickCheckSum = NULL;

    /*
     * Now everything is prepared to store ExecBase at the location 4UL and set
     * it complement in ExecBase structure
     */

    rkprintf("Initializing library...\n");

    *(struct ExecBase **)4 = SysBase;
    SysBase->ChkBase = ~(ULONG)SysBase;

    /* Set up system stack */
    //    tss->ssp = (extmem) ? extmem : locmem;  /* Either in FAST or in CHIP */
//    SysBase->SysStkUpper = (APTR)stack_end;
//    SysBase->SysStkLower = (APTR)&stack[0]; /* 64KB of system stack */

    /* Store memory configuration */
    SysBase->MaxLocMem = (IPTR)0; //locmem;
    SysBase->MaxExtMem = (APTR)0; //extmem;

    /*
     * Initialize exec lists. This is done through information table which consist
     * of offset from begining of ExecBase and type of the list.
     */
    NEWLIST(&SysBase->MemList);
    SysBase->MemList.lh_Type = NT_MEMORY;
    NEWLIST(&SysBase->ResourceList);
    SysBase->ResourceList.lh_Type = NT_RESOURCE;
    NEWLIST(&SysBase->DeviceList);
    SysBase->DeviceList.lh_Type = NT_DEVICE;
    NEWLIST(&SysBase->LibList);
    SysBase->LibList.lh_Type = NT_LIBRARY;
    NEWLIST(&SysBase->PortList);
    SysBase->PortList.lh_Type = NT_MSGPORT;
    NEWLIST(&SysBase->TaskReady);
    SysBase->TaskReady.lh_Type = NT_TASK;
    NEWLIST(&SysBase->TaskWait);
    SysBase->TaskWait.lh_Type = NT_TASK;
    NEWLIST(&SysBase->IntrList);
    SysBase->IntrList.lh_Type = NT_INTERRUPT;
    NEWLIST(&SysBase->SemaphoreList);
    SysBase->SemaphoreList.lh_Type = NT_SIGNALSEM;
    NEWLIST(&SysBase->ex_MemHandlers);

    for (i=0; i<5; i++)
    {
        NEWLIST(&SysBase->SoftInts[i].sh_List);
        SysBase->SoftInts[i].sh_List.lh_Type = NT_SOFTINT;
    }

    /*
     * Exec.library initializer. Prepares exec.library for future use. All
     * lists have to be initialized, some values from ROM are copied.
     */

    SysBase->TaskTrapCode = NULL; //exec_DefaultTrap;
    SysBase->TaskExceptCode = NULL; //exec_DefaultTrap;
    SysBase->TaskExitCode = exec_DefaultTaskExit;
    SysBase->TaskSigAlloc = 0x0000ffff;
    SysBase->TaskTrapAlloc = 0x8000;

    /* Prepare values for execBase (like name, type, pri and other) */

    SysBase->LibNode.lib_Node.ln_Type = NT_LIBRARY;
    SysBase->LibNode.lib_Node.ln_Pri = 0;
    SysBase->LibNode.lib_Node.ln_Name = (char *)exec_name;
    SysBase->LibNode.lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
    SysBase->LibNode.lib_PosSize = sizeof(struct ExecBase);
    SysBase->LibNode.lib_OpenCnt = 1;
    SysBase->LibNode.lib_IdString = (char *)exec_idstring;
    SysBase->LibNode.lib_Version = exec_Version;
    SysBase->LibNode.lib_Revision = exec_Revision;

    SysBase->Quantum = 4;
    SysBase->VBlankFrequency = 50;
    SysBase->PowerSupplyFrequency = 1;

    /* Build the jumptable */
    SysBase->LibNode.lib_NegSize =
        Exec_MakeFunctions(SysBase, Exec_FuncTable, NULL, SysBase);

    SumLibrary((struct Library *)SysBase);

    rkprintf("Adding memory\n");
    struct mb_mmap *mmap;
    uint32_t len = krnGetTagData(KRN_MMAPLength, 0, msg);

    if (len)
    {
        mmap = (struct mb_mmap *)(krnGetTagData(KRN_MMAPAddress, 0, msg));

        while(len >= sizeof(struct mb_mmap))
        {
            if (mmap->type == MMAP_TYPE_RAM)
            {
                uintptr_t addr = (mmap->addr_low | ((intptr_t)mmap->addr_high << 32));
                uintptr_t size = (mmap->len_low | ((intptr_t)mmap->len_high << 32));
                uintptr_t tmp;

                if (addr < (uintptr_t)SysBase)
                {
                    tmp = ((uintptr_t)SysBase +sizeof(struct ExecBase)+ 4095) & ~4095;
                    size -= (tmp-addr);
                    addr = tmp;
                }
                rkprintf("  %012p - %012p\n", addr, addr+size-1);

                if (addr < 0x01000000 && (addr+size) <= 0x01000000)
                {
                    exec_InsertMemory(msg, addr, addr+size-1);
                }
                else if (addr < 0x01000000 && (addr+size) > 0x01000000)
                {
                    exec_InsertMemory(msg, addr, 0x00ffffff);
                    exec_InsertMemory(msg, 0x01000000, addr+size-1);
                }
                else
                {
                    exec_InsertMemory(msg, addr, addr+size-1);
                }
            }

            len -= mmap->size+4;
            mmap = (struct mb_mmap *)(mmap->size + (IPTR)mmap+4);
        }
    }

    
    SumLibrary((struct Library *)SysBase);

    Enqueue(&SysBase->LibList,&SysBase->LibNode.lib_Node);

    SysBase->DebugAROSBase = PrepareAROSSupportBase();
    
    rkprintf("ExecBase=%012p\n", SysBase);

    rkprintf("Leaving supervisor mode\n");
    asm volatile (
            "mov %[user_ds],%%ds\n\t"    // Load DS and ES
            "mov %[user_ds],%%es\n\t"
            "mov %%rsp,%%r12\n\t"
            "pushq %[ds]\n\t"      // SS
            "pushq %%r12\n\t"            // rSP        
            "pushq $0x3002\n\t"         // rFLANGS
            "pushq %[cs]\n\t"      // CS
            "pushq $1f\n\t iretq\n 1:"
            ::[user_ds]"r"(USER_DS),[ds]"i"(USER_DS),[cs]"i"(USER_CS):"r12");
    rkprintf("Done?!\n");
    asm("int $0x20");
        
    /* Scan for valid RomTags */
    SysBase->ResModules = exec_RomTagScanner(msg);

//    InitCode(RTF_SINGLETASK, 0);
//    InitCode(RTF_COLDSTART, 0);

            
    rkprintf("I should never get here...\n");
    while(1);
    return 0;
}


void exec_DefaultTaskExit()
{
    struct ExecBase *SysBase = *(struct ExecBase **)4UL;
    RemTask(SysBase->ThisTask);
}

/* Small delay routine used by exec_cinit initializer */
asm("\ndelay:\t.short   0x00eb\n\tretq");


AROS_LH1(struct ExecBase *, open,
         AROS_LHA(ULONG, version, D0),
         struct ExecBase *, SysBase, 1, Exec)
         {
    AROS_LIBFUNC_INIT

    /* I have one more opener. */
    SysBase->LibNode.lib_OpenCnt++;
    return SysBase;

    AROS_LIBFUNC_EXIT
         }

AROS_LH0(BPTR, close,
         struct ExecBase *, SysBase, 2, Exec)
         {
    AROS_LIBFUNC_INIT

    /* I have one fewer opener. */
    SysBase->LibNode.lib_OpenCnt--;
    return 0;
    AROS_LIBFUNC_EXIT
         }

AROS_LH0I(int, null,
          struct ExecBase *, SysBase, 4, Exec)
          {
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
          }

/*
 * RomTag scanner.
 *
 * This function scans kernel for existing Resident modules. If two modules
 * with the same name are found, the one with higher version or priority wins.
 *
 * After building list of kernel modules, the KickTagPtr and KickMemPtr are
 * checksummed. If checksum is proper and all memory pointed in KickMemPtr may
 * be allocated, then all modules from KickTagPtr are added to RT list
 *
 * Afterwards the proper RomTagList is created (see InitCode() for details) and
 * memory after list and nodes is freed.
 */

struct rt_node
{
    struct Node     node;
    struct Resident *module;
};

IPTR **exec_RomTagScanner(struct TagItem *msg)
{
    struct ExecBase *SysBase = *(struct ExecBase **)4UL;

    struct List     rtList;             /* List of modules */
    UWORD           *ptr = (UWORD*)krnGetTagData(KRN_KernelLowest, 0, msg);  /* Start looking here */
    UWORD           *maxptr = (UWORD*)krnGetTagData(KRN_KernelHighest, 0, msg);
    struct Resident *res;               /* module found */

    int     i;
    IPTR   **RomTag;

    /* Initialize list */
    NEWLIST(&rtList);

    rkprintf("Resident modules (addr: pri version name):\n");

    /* Look in whole kernel for resident modules */
    do
    {
        /* Do we have RTC_MATCHWORD? */
        if (*ptr == RTC_MATCHWORD)
        {
            /* Yes, assume we have Resident */
            res = (struct Resident *)ptr;

            /* Does rt_MatchTag point to Resident? */
            if (res == res->rt_MatchTag)
            {
                /* Yes, it is Resident module */
                struct rt_node  *node;

                /* Check if there is module with such name already */
                node = (struct rt_node*)FindName(&rtList, res->rt_Name);
                if (node)
                {
                    /* Yes, there was such module. It it had lower pri then replace it */
                    if (node->node.ln_Pri <= res->rt_Pri)
                    {
                        /* If they have the same Pri but new one has higher Version, replace */
                        if ((node->node.ln_Pri == res->rt_Pri) &&
                            (node->module->rt_Version < res->rt_Version))
                        {
                            node->node.ln_Pri   = res->rt_Pri;
                            node->module        = res;
                        }
                    }
                }
                else
                {
                    /* New module. Allocate some memory for it */
                    node = (struct rt_node *)
                        AllocMem(sizeof(struct rt_node),MEMF_PUBLIC|MEMF_CLEAR);

                    if (node)
                    {
                        node->node.ln_Name  = res->rt_Name;
                        node->node.ln_Pri   = res->rt_Pri;
                        node->module        = res;

                        Enqueue(&rtList,(struct Node*)node);
                    }
                }
                ptr+=sizeof(struct Resident)/sizeof(UWORD       );
                continue;
            }
        }

        /* Get next address... */
        ptr++;
    } while (ptr < maxptr);

    /*
     * By now we have valid (and sorted) list of kernel resident modules.
     *
     * Now, we will have to analyze used-defined RomTags (via KickTagPtr and
     * KickMemPtr)
     */
#warning "TODO: Implement external modules!"
    /*
     * Everything is done now. Allocate buffer for normal RomTag and convert
     * list to RomTag
     */

    ListLength(&rtList,i);      /* Get length of the list */

    RomTag = AllocMem((i+1)*sizeof(IPTR),MEMF_PUBLIC | MEMF_CLEAR);

    if (RomTag)
    {
        int             j;
        struct rt_node  *n;

        for (j=0; j<i; j++)
        {
            n = (struct rt_node *)RemHead(&rtList);
            rkprintf("+ 0x%012lx: %4d %3d \"%s\"\n",
                n->module,
                n->node.ln_Pri,
                n->module->rt_Version,
                n->node.ln_Name);
            RomTag[j] = (IPTR*)n->module;
            FreeMem(n, sizeof(struct rt_node));
        }
        RomTag[i] = 0;
    }

    return RomTag;
}

/*
    We temporarily redefine kprintf() so we use the real version in case
    we have one of these two fn's called before AROSSupportBase is ready.
 */

#undef kprintf
#undef rkprintf
#undef vkprintf

#define kprintf(x...)
#define rkprintf(x...)
#define vkprintf(x...)

struct Library * PrepareAROSSupportBase(void)
{
    struct ExecBase *SysBase = *(struct ExecBase **)4UL;

    struct AROSSupportBase *AROSSupportBase =
        AllocMem(sizeof(struct AROSSupportBase), MEMF_CLEAR);

    AROSSupportBase->kprintf = (void *)kprintf;
    AROSSupportBase->rkprintf = (void *)rkprintf;
    AROSSupportBase->vkprintf = (void *)vkprintf;

    NEWLIST(&AROSSupportBase->AllocMemList);

#warning "FIXME Add code to read in the debug options"

    return (struct Library *)AROSSupportBase;
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

