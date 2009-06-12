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

#define D(X) X
#define DREL(x)

#define kprintf printf

#define ULONG_PTR intptr_t

#define SECTION_NAME(section_index)         ((char *) (image + sh[eh->shstrndx].offset + sh[section_index].name))
#define SYMBOL_NAME(symtab_index, symbol)   ((char *) (image + sh[sh[symtab_index].link].offset + symbol->name))

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
 * read_block interface. we want to read from files here
 */
static int read_block(FILE *file, long offset, void *dest, long length)
{
  fseek(file, offset, SEEK_SET);
  fread(dest,(size_t)length, 1, file);
  return 1;
}

/*
 * load_block also allocates teh memory
 */
static void *load_block(FILE *file, long offset, long length)
{
  void * dest = malloc(length);
  fseek(file, offset, SEEK_SET);
  fread(dest, (size_t)length, 1, file);
  return dest;
}

static void free_block(void * block)
{
  free(block);
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

/*
 * Get the memory for chunk and load it
 */
static int load_hunk(FILE *file, struct sheader *sh)
{
    void *ptr=(void*)0;
  
    /* empty chunk? Who cares :) */
    if (!sh->size)
	return 1;

    D(kprintf("[elf] Chunk (%d bytes, align=%d) @ ", (unsigned int)sh->size, (unsigned int)sh->addralign));
    ptr_ro = (char *)(((unsigned long)ptr_ro + (unsigned long)sh->addralign - 1) & ~((unsigned long)sh->addralign-1));
    ptr = ptr_ro;
    ptr_ro = ptr_ro + sh->size;
    D(kprintf("%p\n", (unsigned int)ptr));
  
    sh->addr = ptr;
  
    /* copy block of memory from ELF file if it exists */
    if (sh->type != SHT_NOBITS)
	return read_block(file, sh->offset, (void *)((unsigned long)sh->addr), sh->size);
    else
    {
	memset(ptr, 0, sh->size);
	bss_tracker->addr = ptr;
	bss_tracker->len = sh->size;
	bss_tracker++;
    }
    return 1;
}

/* Perform relocations of given section */
static int relocate(struct elfheader *eh, struct sheader *sh, long shrel_idx, ULONG_PTR virt)
{
  struct sheader *shrel    = &sh[shrel_idx];
  struct sheader *shsymtab = &sh[shrel->link];
  struct sheader *toreloc  = &sh[shrel->info];
  
  struct symbol *symtab   = (struct symbol *)((unsigned long)shsymtab->addr);
  struct relo   *rel      = (struct relo *)((unsigned long)shrel->addr);
  char          *section  = (char *)((unsigned long)toreloc->addr);
  
  unsigned int numrel = (unsigned long)shrel->size / (unsigned long)shrel->entsize;
  unsigned int i;
  
  DREL(kprintf("[elf] performing %d relocations, virtual address %p\n", numrel, virt));
  
  for (i=0; i<numrel; i++, rel++)
  {
	struct symbol *sym = &symtab[ELF32_R_SYM(rel->info)];
	unsigned long *p = (unsigned long *)&section[rel->offset];
	ULONG_PTR s;
	const char * name = (char *)((unsigned long)sh[shsymtab->link].addr) + sym->name;
	switch (sym->shindex)
	{
	case SHN_UNDEF:
	    DREL(kprintf("[elf] Undefined symbol '%s' while relocating the section '%s'\n",
				  (char *)((unsigned long)sh[shsymtab->link].addr) + sym->name,
				  (char *)((unsigned long)sh[eh->shstrndx].addr) + toreloc->name));
	    return 0;
		
	case SHN_COMMON:
	    DREL(kprintf("[elf] COMMON symbol '%s' while relocating the section '%s'\n",
				  (char *)((unsigned long)sh[shsymtab->link].addr) + sym->name,
				  (char *)((unsigned long)sh[eh->shstrndx].addr) + toreloc->name));
		
	    return 0;
		
	case SHN_ABS:
            s = sym->value;
	    break;
		
	default:
	    s = (unsigned long)sh[sym->shindex].addr + sym->value;
	}

	s += virt;

        DREL(printf("[elf] Relocating symbol "));
        DREL(if (sym->name) printf("%s", name); else printf("<unknown>"));
        DREL(printf("type "));
	switch (ELF32_R_TYPE(rel->info))
	{
	case R_386_32: /* 32bit absolute */
            DREL(printf("R_386_32"));
	    *p += s;
        if (*p < kbase) {
            exit(0);
        }

	    break;
		
	case R_386_PC32: /* 32bit PC relative */
            DREL(printf("R_386_PC32"));
	    *p += (s - (ULONG_PTR)p);
	    break;
		
	case R_386_NONE:
            DREL(printf("R_386_NONE"));
	    break;
		
	default:
	    printf("[elf] Unrecognized relocation type %d %d\n", i, (unsigned int)ELF32_R_TYPE(rel->info));
	    return 0;
	}
	DREL(printf(" -> 0x%p\n", *p));
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

 #if 0 
    /* For every loaded section perform the relocations */
    for (i=0; i < eh->shnum; i++)
    {
	if ((sh[i].type == SHT_RELA || sh[i].type == SHT_REL) && sh[sh[i].info].addr)
	{
	  sh[i].addr = load_block(file, sh[i].offset, sh[i].size);
	  if (!sh[i].addr || !relocate(&eh, sh, i, virt))
	  {
		kprintf("[elf] Relocation error!\n");
	  }
	  free_block(sh[i].addr);
	}
	else if (sh[i].type == SHT_SYMTAB || sh[i].type == SHT_STRTAB)
	  free_block(sh[i].addr);
    }
    free_block(sh);
#endif
    return 1;
}

