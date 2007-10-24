#include <aros/config.h>

#if AROS_STACK_PROTECTOR

#include <aros/debug.h>

#include <exec/alerts.h>
#include <proto/exec.h>

void __stack_chk_fail(void) {
    kprintf("stack smashing detected\n");
    Alert(AT_DeadEnd);
}

#endif
