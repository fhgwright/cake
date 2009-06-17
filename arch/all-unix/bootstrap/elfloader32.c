/*
 Copyright (C) 2006-2009 The AROS Development Team. All rights reserved.
 $Id$
 
 Desc: ELF32 loader extracted from our internal_load_seg_elf in dos.library.
 Lang: English
 */

#include "elfloader32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#define D(x)
#define DREL(x)

#define kprintf printf

#define ULONG_PTR intptr_t

#define SECTION_NAME(section_index)         ((char *) (image + sh[eh->shstrndx].offset + sh[section_index].name))
#define SYMBOL_NAME(symtab_index, symbol)   ((char *) (symbol->name ? (image + sh[sh[symtab_index].link].offset + symbol->name) : "<anonymous>"))

/*
 * These two pointers are used by the ELF loader to claim for memory ranges for both
 * the RW sections (.data, .bss and such) and RO sections (.text, .rodata....) of executable.
 */
char *kbase;
char *ptr_ro;
char *entry = NULL;

struct _bss_tracker {
  void *addr;
  size_t len;
} *bss_tracker;

void *kernel_lowest()
{
    return kbase;
}

void *kernel_highest()
{
    return ptr_ro - 1;
}

void *kernel_entry()
{
    return entry;
}

void set_base_address(void *tracker)
{
  D(printf("[elf] set_base_address %p\n", tracker));
  bss_tracker = (struct _bss_tracker *)tracker;
}

/*
 * Test for correct ELF header here
 */
static int check_header(struct elfheader *eh)
{
  if
    (
	 eh->ident[0] != 0x7f ||
	 eh->ident[1] != 'E'  ||
	 eh->ident[2] != 'L'  ||
	 eh->ident[3] != 'F'
	 )
  {
	D(kprintf("[elf] Not an ELF object\n"));
	return 0;
  }
  
  if (eh->type != ET_REL || eh->machine != EM_386)
  {
	D(kprintf("[elf] Wrong object type or wrong architecture\n"));
	return 0;
  }
  return 1;
}

int load_elf_image(void *image, void *memory) {
    struct elfheader *eh;
    struct sheader *sh;
    int i;
    uint32_t alloc = 0;
    ULONG_PTR virt = 0;

    D(kprintf("[elf] Loading ELF module from virtual address %p\n", virt));

    eh = (struct elfheader *) image;
    if (!check_header(eh)) {
        fprintf(stderr, "[elf] this is not an ELF image\n");
        return -1;
    }

    sh = (struct sheader *) (image + eh->shoff);

    for (i = 0; i < eh->shnum; i++) {
        if (sh[i].flags & SHF_ALLOC) {
            alloc = (alloc + sh[i].addralign - 1) & ~(sh[i].addralign - 1);
            sh[i].addr = (void *) memory + alloc;

            D(printf("[elf] allocating 0x%x bytes at 0x%x (alignment 0x%x) for section '%s'\n", sh[i].size, sh[i].addr, sh[i].addralign, SECTION_NAME(i)));

            switch (sh[i].type) {
                case SHT_PROGBITS:
                    D(printf("[elf] section is SHT_PROGBITS, copying 0x%x bytes\n", sh[i].size));
                    memcpy(sh[i].addr, image + sh[i].offset, sh[i].size);

                    if (sh[i].flags & SHF_EXECINSTR && entry == NULL) {
                        entry = sh[i].addr;
                        D(printf("[elf] first executable section, entry point is 0x%x\n", entry));
                    }

                    break;

                case SHT_NOBITS:
                    D(printf("[elf] section is SHT_NOBITS, clearing 0x%x bytes\n", sh[i].size));
                    memset(sh[i].addr, 0, sh[i].size);
                    break;

                default:
                    fprintf(stderr, "[elf] no support to allocate for type 0x%x section\n", sh[i].type);
                    return -1;
            }

            alloc += sh[i].size;
        }
    }

    for (i = 0; i < eh->shnum; i++) {
        if (sh[i].type == SHT_REL || sh[i].type == SHT_RELA) {
            if(sh[sh[i].info].addr == NULL) {
                D(printf("[elf] section '%s' has relocations for section '%s', but that section has no allocation, skipping it\n", SECTION_NAME(i), SECTION_NAME(sh[i].info)));
                continue;
            }

            int nrelocs = sh[i].size / sh[i].entsize;

            D(printf("[elf] applying %d relocations in section '%s' to section '%s' using symbol table in section '%s'\n", nrelocs, SECTION_NAME(i), SECTION_NAME(sh[i].info), SECTION_NAME(sh[i].link)));

            int j;
            for (j = 0; j < nrelocs; j++) {
                struct relo *rel = image + sh[i].offset + sizeof(struct relo) * j;
                struct symbol *sym = image + sh[sh[i].link].offset + sizeof(struct symbol) * ELF32_R_SYM(rel->info);
                uint32_t *src = image + sh[sh[i].info].offset + rel->offset;
                uint32_t *dst = sh[sh[i].info].addr + rel->offset;
                uint32_t s;

                switch (sym->shindex) {
                    case SHN_UNDEF:
                        fprintf(stderr, "[elf] undefined symbol '%s' in section '%s', relocation failed\n", SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;

                    case SHN_COMMON:
                        fprintf(stderr, "[elf] undefined symbol '%s' in section '%s', relocation failed\n", SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;

                    case SHN_ABS:
                        if (strcmp(SYMBOL_NAME(sh[i].link, sym), "SysBase") == 0) {
                            fprintf(stderr, "[elf] absolute SysBase relocation wanted in section '%s'. that's bad for kernel code, and may cause a crash\n", SECTION_NAME(sh[i].info));
                            s = 0;
                        }
                        else
                            s = sym->value;
                        break;

                    default:
                        s = (uint32_t) sh[sym->shindex].addr + sym->value;
                        break;
                }

                switch (ELF32_R_TYPE(rel->info)) {
                    case R_386_32:
                        *dst = *src + s;
                        break;

                    case R_386_PC32:
                        *dst = *src + s - (uint32_t) dst;
                        break;

                    case R_386_NONE:
                        break;

                    default:
                        fprintf(stderr, "[elf] unknown relocation type 0x%02x for symbol '%s' in section '%s', relocation failed\n", ELF32_R_TYPE(rel->info), SYMBOL_NAME(sh[i].link, sym), SECTION_NAME(sh[i].info));
                        return -1;
                }

                DREL(printf("[elf] relocated symbol '%s' type %s value %p at %p\n", SYMBOL_NAME(sh[i].link, sym),
                                                                                        ELF32_R_TYPE(rel->info) == R_386_32   ? "R_386_32"   :
                                                                                        ELF32_R_TYPE(rel->info) == R_386_PC32 ? "R_386_PC32" :
                                                                                                                        "<unknown>",
                                                                                        *dst, dst));
            }
        }
    }

    return 0;
}

