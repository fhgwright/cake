#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <stdint.h>

int load_elf_image(void *image, void *memory, uint32_t virt, void **start, void **end, void **entry);

#endif
