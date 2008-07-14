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

struct segment {
    int                 index;
    ULONG               size;
    BPTR                next;
    char                data[0];
} __attribute__((packed));

#define BPTR_TO_SEGMENT(bptr)  (void *) ((struct segment *)((char *) BADDR(bptr) - offsetof(struct segment, next)))
#define SEGMENT_TO_BPTR(thing) MKBADDR(&thing->next)

#define SHINDEX(n) \
    ((n) < SHN_LORESERVE ? (n) : ((n) <= SHN_HIRESERVE ? 0 : (n) - (SHN_HIRESERVE + 1 - SHN_LORESERVE)))

#define SHNUM(i) \
    ((i) < SHN_LORESERVE ? (i) : (i) + (SHN_HIRESERVE + 1 - SHN_LORESERVE))


/* these macros are taken from binutils */

/* .tbss is special.  It doesn't contribute memory space to normal
   segments and it doesn't take file space in normal segments.  */
#define ELF_SECTION_SIZE(sec_hdr, segment)                      \
   (((sec_hdr->sh_flags & SHF_TLS) == 0                         \
     || sec_hdr->sh_type != SHT_NOBITS                          \
     || segment->p_type == PT_TLS) ? sec_hdr->sh_size : 0)

/* Decide if the given sec_hdr is in the given segment.  PT_TLS segment
   contains only SHF_TLS sections.  Only PT_LOAD and PT_TLS segments
   can contain SHF_TLS sections.  */
#define ELF_IS_SECTION_IN_SEGMENT(sec_hdr, segment)                     \
  (((((sec_hdr->sh_flags & SHF_TLS) != 0)                               \
     && (segment->p_type == PT_TLS                                      \
         || segment->p_type == PT_LOAD))                                \
    || ((sec_hdr->sh_flags & SHF_TLS) == 0                              \
        && segment->p_type != PT_TLS))                                  \
   /* Any section besides one of type SHT_NOBITS must have a file       \
      offset within the segment.  */                                    \
   && (sec_hdr->sh_type == SHT_NOBITS                                   \
       || (sec_hdr->sh_offset >= segment->p_offset                      \
           && (sec_hdr->sh_offset + ELF_SECTION_SIZE(sec_hdr, segment)  \
               <= segment->p_offset + segment->p_filesz)))              \
   /* SHF_ALLOC sections must have VMAs within the segment.  */         \
   && ((sec_hdr->sh_flags & SHF_ALLOC) == 0                             \
       || (sec_hdr->sh_addr >= segment->p_vaddr                         \
           && (sec_hdr->sh_addr + ELF_SECTION_SIZE(sec_hdr, segment)    \
               <= segment->p_vaddr + segment->p_memsz))))

/* Decide if the given sec_hdr is in the given segment in memory.  */
#define ELF_IS_SECTION_IN_SEGMENT_MEMORY(sec_hdr, segment)      \
  (ELF_SECTION_SIZE(sec_hdr, segment) > 0                       \
   && ELF_IS_SECTION_IN_SEGMENT (sec_hdr, segment))


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
        //D(bug("[elf] unsupported object type %d\n", h->eh.e_type));
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

static struct segment *load_segment (BPTR                file,
                                     Elf32_Phdr         *ph,
                                     int                 index,
                                     SIPTR              *helpers,
                                     struct DosLibrary  *DOSBase)
{
    struct segment *segment;

    if (ph[index].p_memsz < ph[index].p_filesz) {
        SetIoErr(ERROR_BAD_HUNK);
        return NULL;
    }

    if (ph[index].p_memsz == 0)
        return NULL;
    
    if (!(segment = HELPER_ALLOC(helpers, sizeof(struct segment) + ph[index].p_memsz, MEMF_ANY))) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return NULL;
    }

    segment->size = sizeof(struct segment) + ph[index].p_memsz;
    segment->next = 0;

    segment->index = index;

    ph[index].p_paddr = segment->data;

    memset(segment->data + ph[index].p_filesz, 0, ph[index].p_memsz - ph[index].p_filesz);
        
    if (!read_block(file, ph[index].p_offset, (APTR) segment->data, ph[index].p_filesz, helpers, DOSBase)) {
        HELPER_FREE(helpers, segment, segment->size);
        return NULL;
    }

    return segment;
}

BPTR InternalLoadSeg_ELF_New (BPTR               file,
                              BPTR               table     __unused,
                              SIPTR             *helpers,
                              SIPTR             *stack     __unused,
                              struct MinList    *seginfos,
                              struct DosLibrary *DOSBase)
{
    BPTR seglist = 0;
    struct segment *last;
    BOOL have_trampoline = FALSE;
    elf_header *h = NULL;
    Elf32_Phdr *ph = NULL;
    Elf32_Shdr *sh = NULL;
    Elf32_Shdr *symtab_shndx = NULL;
    char *strtab = NULL;
    
    if (!(h = load_header(file, helpers, DOSBase)))
        goto _loadseg_fail;

    if (!(ph = load_block(file, h->eh.e_phoff, h->eh.e_phnum * h->eh.e_phentsize, helpers, DOSBase)))
        goto _loadseg_fail;

    if (!(sh = load_block(file, h->eh.e_shoff, h->shnum * h->eh.e_shentsize, helpers, DOSBase)))
        goto _loadseg_fail;

    if (!(strtab = load_block(file, sh[SHINDEX(h->shstrndx)].sh_offset, sh[SHINDEX(h->shstrndx)].sh_size, helpers, DOSBase)))
        goto _loadseg_fail;

    D(bug("[elf] program headers:\n"));

    int i;
    for (i = 0; i < h->eh.e_phnum; i++) {
        D(bug("  %d: type %d off 0x%p vaddr 0x%p paddr 0x%p filesz 0x%04x memsz 0x%04x flags 0x%p align 0x%02x\n", i, ph[i].p_type, ph[i].p_offset, ph[i].p_vaddr, ph[i].p_paddr, ph[i].p_filesz, ph[i].p_memsz, ph[i].p_flags, ph[i].p_align));

        if (ph[i].p_type == PT_LOAD) {
            struct segment *segment;

            if (!(segment = load_segment(file, ph, i, helpers, DOSBase)))
                goto _loadseg_fail;

            if (!seglist)
                seglist = SEGMENT_TO_BPTR(segment);
            else
                last->next = SEGMENT_TO_BPTR(segment);
            last = segment;

            D(bug("[elf] loaded PT_LOAD segment\n"));

            if (ph[i].p_flags & PF_X) {
                struct segment *trampoline;

                if (have_trampoline) {
                    D(bug("[elf] multiple executable segments found, aborting\n"));
                    SetIoErr(ERROR_BAD_HUNK);
                    goto _loadseg_fail;
                }

                D(bug("[elf] segment is executable, creating trampoline hunk\n"));

                if (!((h->eh.e_entry >= ph[i].p_vaddr) && (h->eh.e_entry <= (ph[i].p_vaddr + ph[i].p_memsz)))) {
                    D(bug("[elf] entry point 0x%p is outside segment (0x%p-0x%p), aborting\n", h->eh.e_entry, ph[i].p_vaddr, ph[i].p_vaddr + ph[i].p_memsz));
                    SetIoErr(ERROR_BAD_HUNK);
                    goto _loadseg_fail;
                }

                if (!(trampoline = HELPER_ALLOC(helpers, sizeof(struct segment) + sizeof(struct FullJumpVec), MEMF_ANY))) {
                    SetIoErr(ERROR_NO_FREE_STORE);
                    goto _loadseg_fail;
                }

                trampoline->size = sizeof(struct segment) + sizeof(struct FullJumpVec);

                trampoline->next = seglist;
                seglist = SEGMENT_TO_BPTR(trampoline);

                trampoline->index = -1;

                __AROS_SET_FULLJMP((struct FullJumpVec *) &trampoline->data,
                                   (ULONG) h->eh.e_entry + (ULONG) ph[i].p_paddr - (ULONG) ph[i].p_vaddr);
                
                D(bug("[elf] trampoline target set to 0x%p\n", (ULONG) h->eh.e_entry + (ULONG) ph[i].p_paddr - (ULONG) ph[i].p_vaddr));

                have_trampoline = TRUE;
            }
        }
    }

    for (i = 0; i < h->shnum; i++) {
        if (sh[i].sh_type == SHT_SYMTAB || sh[i].sh_type == SHT_STRTAB || sh[i].sh_type == SHT_SYMTAB_SHNDX) {
            if (!(sh[i].sh_addr = load_block(file, sh[i].sh_offset, sh[i].sh_size, helpers, DOSBase)))
                goto _loadseg_fail;

            if (sh[i].sh_type == SHT_SYMTAB_SHNDX) {
                if (symtab_shndx == NULL) {
                    symtab_shndx = &sh[i];
                    D(bug("[elf] symtab shndx table found at section header %d\n", i));
                }
                else
                    D(bug("[elf] file contains multiple symtab shndx tables, only using the first one\n"));
            }
        }
    }

    D(bug("[elf] section headers:\n"));

    for (i = 0; i < h->shnum; i++) {
        D(bug("  %d: name '%s' type %d flags 0x%p addr 0x%p offset 0x%p size 0x%04x link %d info 0x%p addralign 0x%02x entsize %d\n", i, strtab + sh[i].sh_name, sh[i].sh_type, sh[i].sh_flags, sh[i].sh_addr, sh[i].sh_offset, sh[i].sh_size, sh[i].sh_link, sh[i].sh_info, sh[i].sh_addralign, sh[i].sh_entsize));

        if (sh[i].sh_flags & SHF_ALLOC) {
            D(bug("[elf] found SHT_ALLOC section\n"));

            struct segment *target = BPTR_TO_SEGMENT(seglist);
            while (target && (target->index < 0 || !(ELF_IS_SECTION_IN_SEGMENT_MEMORY((&sh[i]), (&ph[target->index])))))
                target = BPTR_TO_SEGMENT(target->next);

            if (target) {
                D(bug("[elf] section '%s' allocation appears in program header %d\n", strtab + sh[i].sh_name, target->index));
            }
        }
    }

#if 0
    for (i = 0; i < h->shnum; i++) {

        /* FIXME: RELA everywhere else */
        if (sh[i].type == SHT_REL) {
            sh[i].addr = load_block(file, sh[i].sh_offset, sh[i].sh_size, helpers, DOSBase);
            if (!sh[i].addr)
                goto _loadseg_fail;

            if (!relocate(ph, sh[i].sh_addr, sh[i].sh_info)) {
                HELPER_FREE(helpers, sh[i].sh_addr, sh[i].sh_size);
                goto _loadseg_fail;
            }
            
            HELPER_FREE(helpers, sh[i].sh_addr, sh[i].sh_size);
            sh[i].addr = NULL;
        }
    }
#endif

    goto _loadseg_end;

_loadseg_fail:
    if (seglist)
        InternalUnLoadSeg(seglist, (VOID_FUNC) helpers[2]);

    if (sh)
        for (i = 0; i < h->shnum; i++)
            if (sh[i].sh_addr)
                HELPER_FREE(helpers, sh[i].sh_addr, sh[i].sh_size);

_loadseg_end:
    if (strtab)
        HELPER_FREE(helpers, strtab, sh[SHINDEX(h->shstrndx)].sh_size);

    if (sh)
        HELPER_FREE(helpers, sh, h->shnum * h->eh.e_shentsize);

    if (ph)
        HELPER_FREE(helpers, ph, h->eh.e_phnum * h->eh.e_phentsize);

    if (h)
        HELPER_FREE(helpers, h, sizeof(elf_header));

    return seglist;
}
