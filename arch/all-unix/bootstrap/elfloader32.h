#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <stdint.h>

extern uint32_t elf_count_allocation (void *image);
extern int elf_load_image (void *image, void *memory, uint32_t virt, void **start, void **end, void **entry);

#endif
