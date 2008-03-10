#include <exec/types.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdlib.h>

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

struct hunk {
    ULONG size;
    BPTR  next;
    char  data[0];
} __attribute__((packed));

#define BPTR2HUNK(bptr) ((struct hunk *) ((char *) BADDR(bptr) - offsetof(struct hunk, next)))
#define HUNK2BPTR(hunk) MKBADDR(&hunk->next)

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

static BOOL load_hunk (BPTR                file,
                       BPTR              **next_hunk_ptr,
                       Elf32_Phdr         *ph,
                       SIPTR              *helpers,
                       struct DosLibrary  *DOSBase)
{
    struct hunk *hunk;

    if (ph->p_memsz < ph->p_filesz) {
        SetIoErr(ERROR_BAD_HUNK);
        return FALSE;
    }

    if (ph->p_memsz == 0)
        return TRUE;
    
    if (!(hunk = HELPER_ALLOC(helpers, ph->p_memsz + sizeof(struct hunk), MEMF_ANY))) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return FALSE;
    }

    hunk->size = ph->p_memsz + sizeof(struct hunk);

    ph->p_paddr = hunk->data;

    hunk->next = BPTR2HUNK(*next_hunk_ptr)->next;
    BPTR2HUNK(*next_hunk_ptr)->next = HUNK2BPTR(hunk);
    *next_hunk_ptr = HUNK2BPTR(hunk);

    memset(hunk->data + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        
    return read_block(file, ph->p_offset, (APTR) hunk->data, ph->p_filesz, helpers, DOSBase);
}

static elf_header *load_header (BPTR               file,
                                SIPTR             *helpers,
                                struct DosLibrary *DOSBase)
{
    elf_header *h = HELPER_ALLOC(helpers, sizeof(elf_header), MEMF_ANY);

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
    HELPER_FREE(helpers, h, sizeof(elf_header));
    return NULL;
}

BPTR InternalLoadSeg_ELF_New (BPTR               file,
                              BPTR               table     __unused,
                              SIPTR             *helpers,
                              SIPTR             *stack     __unused,
                              struct MinList    *seginfos,
                              struct DosLibrary *DOSBase)
{
    BPTR  hunks     = 0;
    BPTR *next_hunk = &hunks;
    BOOL have_exec_segment = FALSE;
    elf_header *h = NULL;
    Elf32_Phdr *ph = NULL;
    
    if (!(h = load_header(file, helpers, DOSBase)))
        goto _loadseg_fail;

    if (!(ph = load_block(file, h->eh.e_phoff, h->eh.e_phnum * h->eh.e_phentsize, helpers, DOSBase)))
        goto _loadseg_fail;

    D(bug("[elf] program headers:\n"));

    int i;
    for (i = 0; i < h->eh.e_phnum; i++) {
        D(bug("  %d: type %d off 0x%p vaddr 0x%p paddr 0x%p filesz %d memsz %d flags 0x%p align %d\n", i, ph[i].p_type, ph[i].p_offset, ph[i].p_vaddr, ph[i].p_paddr, ph[i].p_filesz, ph[i].p_memsz, ph[i].p_flags, ph[i].p_align));

        if (ph[i].p_type == PT_LOAD) {
            if (!load_hunk(file, &next_hunk, &ph[i], helpers, DOSBase))
                goto _loadseg_fail;

            D(bug("[elf] loaded PT_LOAD segment\n"));

            if (ph[i].p_flags | PF_X) {
                struct hunk *hunk;

                D(bug("[elf] segment is executable, creating trampoline hunk\n"));

                if (have_exec_segment) {
                    D(bug("[fat] multiple executable segments found, aborting\n"));
                    SetIoErr(ERROR_BAD_HUNK);
                    goto _loadseg_fail;
                }

                have_exec_segment = TRUE;

                if (!((h->eh.e_entry >= ph[i].p_vaddr) && (h->eh.e_entry <= (ph[i].p_vaddr + ph[i].p_memsz)))) {
                    D(bug("[fat] entry point 0x%p is outside segment (0x%p-0x%p), aborting\n", h->eh.e_entry, ph[i].p_vaddr, ph[i].p_vaddr + ph[i].p_memsz));
                    SetIoErr(ERROR_BAD_HUNK);
                    goto _loadseg_fail;
                }

                if (!(hunk = HELPER_ALLOC(helpers, sizeof(struct FullJumpVec) + sizeof(struct hunk), MEMF_ANY | MEMF_CLEAR))) {
                    SetIoErr(ERROR_NO_FREE_STORE);
                    goto _loadseg_fail;
                }

                hunk->size = sizeof(struct FullJumpVec) + sizeof(struct hunk);
                hunk->next = BPTR2HUNK(hunks)->next;
                BPTR2HUNK(hunks)->next = HUNK2BPTR(hunk);

                __AROS_SET_FULLJMP((struct FullJumpVec *) hunk->data,
                                   (ULONG) h->eh.e_entry + (ULONG) ph[i].p_paddr - (ULONG) ph[i].p_vaddr);
                
                D(bug("[elf] trampoline target set to 0x%p\n", (ULONG) h->eh.e_entry + (ULONG) ph[i].p_paddr - (ULONG) ph[i].p_vaddr));
            }
        }
    }

    goto _loadseg_end;

_loadseg_fail:
    if (hunks) {
        InternalUnLoadSeg(hunks, (VOID_FUNC) helpers[2]);
        hunks = 0;
    }

_loadseg_end:
    if (ph)
        HELPER_FREE(helpers, ph, h->eh.e_phnum * h->eh.e_phentsize);

    if (h)
        HELPER_FREE(helpers, h, sizeof(elf_header));

    return hunks;
}
