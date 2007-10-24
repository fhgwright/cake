#include <aros/config.h>

#if AROS_STACK_PROTECTOR

#include <exec/execbase.h>
#include <exec/alerts.h>
#include <proto/exec.h>
#include <aros/arossupportbase.h>

void __stack_chk_fail(void) {
    struct AROSSupportBase *AROSSupportBase = (struct AROSSupportBase *) SysBase->DebugData;

    AROSSupportBase->kprintf("[kernel] stack smashing detected\n");

    Alert(AG_BadParm);
}

#endif
