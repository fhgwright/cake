#include <aros/config.h>

#if AROS_STACK_PROTECTOR

/*
#include <stdio.h>
#include <stdlib.h>
*/

void __stack_chk_fail(void) {
    asm("int $3");
    /*
    fprintf(stderr, "stack smashing detected\n");
    exit(10);
    */
}

#endif
