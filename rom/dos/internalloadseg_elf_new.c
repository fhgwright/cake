#include <exec/types.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "dos_intern.h"

#define DEBUG 1
#include <aros/debug.h>

#include "elf.h"

/* FIXME: 64bit */
typedef struct elf_header {
    Elf32_Ehdr  eh;
    Elf32_Word  shnum;
    Elf32_Word  shstrndx;
} elf_header;

#define HELPER_READ(helpers, file, buf, size) \
    AROS_CALL3(LONG, helpers[0],              \
               AROS_LCA(BPTR, file, D1),      \
               AROS_LCA(APTR, buf,  D2),      \
               AROS_LCA(LONG, size, D3),      \
               struct DosLibrary *, DOSBase)

#define HELPER_ALLOC(helpers, size, flags) \
    AROS_CALL2(void *, helpers[1],         \
               AROS_LCA(ULONG, size,  D0), \
               AROS_LCA(ULONG, flags, D1), \
               struct ExecBase *, SysBase) \

#define HELPER_FREE(helpers, addr, size)     \
    AROS_CALL2NR(void, helpers[2],           \
                 AROS_LCA(void *, addr, A1), \
                 AROS_LCA(ULONG,  size, D0), \
                 struct ExecBase *, SysBase) \

static BOOL read_block (BPTR               file,
                        LONG               offset,
                        APTR              *bufptr,
                        LONG               size,
                        SIPTR             *helpers,
                        struct DosLibrary *DOSBase)
{
    UBYTE *buffer = (UBYTE *) bufptr;
    LONG subsize;

    if (Seek(file, offset, OFFSET_BEGINNING) < 0)
        return FALSE;

    while (size) {
        subsize = HELPER_READ(helpers, file, buffer, size);
        if (subsize < 0)
            return FALSE;

        if (subsize == 0) {
            SetIoErr(ERROR_BAD_HUNK);

            return FALSE;
        }

        buffer += subsize;
        size -= subsize;
    }

    return TRUE;
}

static void *load_block (BPTR               file,
                         ULONG              offset,
                         ULONG              size,
                         SIPTR             *helpers,
                         struct DosLibrary *DOSBase)
{
    void *block = HELPER_ALLOC(helpers, size, MEMF_ANY);
    if (block == NULL) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return NULL;
    }

    if (!read_block(file, offset, block, size, helpers, DOSBase)) {
        HELPER_FREE(helpers, block, size);
        return NULL;
    }

    return block;
}

static elf_header *load_header (BPTR               file,
                                SIPTR             *helpers,
                                struct DosLibrary *DOSBase)
{
    elf_header *h = AllocVec(sizeof(elf_header), 0);

    D(bug("[elf] loading header\n"));

    if (!read_block(file, 0, (APTR) &h->eh, sizeof(Elf32_Ehdr), helpers, DOSBase))
        return NULL;

    if (h->eh.e_ident[EI_MAG0] != 0x7f || h->eh.e_ident[EI_MAG1] != 'E'  ||
        h->eh.e_ident[EI_MAG2] != 'L'  || h->eh.e_ident[EI_MAG3] != 'F')
    {
        D(bug("[elf] incorrect magic number; this is not an ELF object\n"));
        goto _header_fail;
    }

    /* XXX: 64bit */
    if (h->eh.e_ident[EI_CLASS] != ELFCLASS32) {
        D(bug("[elf] unsupported ELF class %d\n", h->eh.e_ident[EI_CLASS]));
        goto _header_fail;
    }

    /* XXX: bigendian */
    if (h->eh.e_ident[EI_DATA] != ELFDATA2LSB) {
        D(bug("[elf] unsupported ELF encoding %d\n", h->eh.e_ident[EI_DATA]));
        goto _header_fail;
    }

    if (h->eh.e_ident[EI_VERSION] != EV_CURRENT || h->eh.e_version != EV_CURRENT) {
        D(bug("[elf] unsupported ELF version %d\n", h->eh.e_ident[EI_VERSION]));
        goto _header_fail;
    }

    if (h->eh.e_ident[EI_OSABI] != ELFOSABI_NONE) {
        D(bug("[elf] unsupported OS ABI %d\n", h->eh.e_ident[EI_OSABI]));
        goto _header_fail;
    }

    /* XXX: ET_REL, ET_EXEC */
    if (h->eh.e_type != ET_DYN) {
        D(bug("[elf] unsupported object type %d\n", h->eh.e_type));
        goto _header_fail;
    }

    /* XXX: non-i386 */
    if (h->eh.e_machine != EM_386) {
        D(bug("[elf] unsupported machine type %d\n", h->eh.e_machine));
        goto _header_fail;
    }

    h->shnum    = h->eh.e_shnum;
    h->shstrndx = h->eh.e_shstrndx;

    if (h->eh.e_shnum == 0 || h->eh.e_shstrndx == SHN_XINDEX) {
        if (h->eh.e_shoff == 0) {
            D(bug("[elf] header wants extended section count or string table index, but has no section header table\n"));
            goto _header_fail;
        }

        Elf32_Shdr sh;
        if (!read_block(file, h->eh.e_shoff, (APTR) &sh, sizeof(Elf32_Shdr), helpers, DOSBase))
            goto _header_fail;

        if (h->shnum == 0)
            h->shnum = sh.sh_size;

        if (h->shstrndx == SHN_XINDEX)
            h->shstrndx = sh.sh_link;

        if (h->eh.e_shnum == 0 || h->eh.e_shstrndx == SHN_XINDEX) {
            D(bug("[elf] section count or string table index invalid even after checking the first section header, failing\n"));
            goto _header_fail;
        }
    }

    D(bug("[elf] loaded header:\n"));
    D(bug("    entry point: 0x%p\n", h->eh.e_entry));
    D(bug("    program headers:\n"));
    D(bug("           table offset: 0x%p\n", h->eh.e_phoff));
    D(bug("         num of headers: %d\n", h->eh.e_phnum));
    D(bug("             entry size: 0x%04x\n", h->eh.e_phentsize));
    D(bug("    section headers:\n"));
    D(bug("           table offset: 0x%p\n", h->eh.e_shoff));
    D(bug("         num of headers: %d\n", h->shnum));
    D(bug("             entry size: 0x%04x\n", h->eh.e_shentsize));
    D(bug("     string table index: %d\n", h->shstrndx));

    return h;

_header_fail:
    SetIoErr(ERROR_NOT_EXECUTABLE);
    FreeVec(h);
    return NULL;
}

BPTR InternalLoadSeg_ELF_New (BPTR               file,
                              BPTR               table     __unused,
                              SIPTR             *helpers,
                              SIPTR             *stack     __unused,
                              struct MinList    *seginfos,
                              struct DosLibrary *DOSBase)
{
    elf_header *h;
    Elf32_Phdr *ph;
    
    if (!(h = load_header(file, helpers, DOSBase)))
        goto _loadseg_fail;

    if (!(ph = load_block(file, h->eh.e_phoff, h->eh.e_phnum * h->eh.e_phentsize, helpers, DOSBase)))
        goto _loadseg_fail;

_loadseg_fail:
    if (h)
        FreeVec(h);

    return 0;
}
