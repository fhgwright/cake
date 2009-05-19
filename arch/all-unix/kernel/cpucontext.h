#define _GNU_SOURCE 1
#include <ucontext.h>

#ifdef __i386__

#define PRINT_CPUCONTEXT(ctx) \
    kprintf ("    ESP=%08lx  EBP=%08lx  EIP=%08lx\n" \
             "    EAX=%08lx  EBX=%08lx  ECX=%08lx  EDX=%08lx\n" \
             "    EDI=%08lx  ESI=%08lx  EFLAGS=%08lx\n" \
             , ctx->uc_mcontext.gregs[REG_ESP] \
             , ctx->uc_mcontext.gregs[REG_EIP] \
             , ctx->uc_mcontext.gregs[REG_EAX] \
             , ctx->uc_mcontext.gregs[REG_EBX] \
             , ctx->uc_mcontext.gregs[REG_ECX] \
             , ctx->uc_mcontext.gregs[REG_EDX] \
             , ctx->uc_mcontext.gregs[REG_EDI] \
             , ctx->uc_mcontext.gregs[REG_ESI] \
             , ctx->uc_mcontext.gregs[REG_ESP] \
             , ctx->uc_mcontext.gregs[REG_EFL] \
    );

#define PREPARE_INITIAL_CONTEXT(ctx, sp, pc) ctx->Ebp = 0;             \
                         ctx->Eip = (IPTR)pc;         \
                         ctx->Esp = (IPTR)sp;         \
                         ctx->ContextFlags = CONTEXT_CONTROL;

#define REG_SAVE_VAR DWORD SegCS_Save, SegSS_Save

#define CONTEXT_INIT_FLAGS(ctx) (ctx)->ContextFlags = CONTEXT_FULL|CONTEXT_INTEGER|CONTEXT_FLOATING_POINT|CONTEXT_DEBUG_REGISTERS|CONTEXT_EXTENDED_REGISTERS

#define CONTEXT_SAVE_REGS(ctx)    SegCS_Save = (ctx)->SegCs; \
                          SegSS_Save = (ctx)->SegSs

#define CONTEXT_RESTORE_REGS(ctx) (ctx)->SegCs = SegCS_Save; \
                  (ctx)->SegSs = SegSS_Save; \
                  (ctx)->ContextFlags &= CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_FLOATING_POINT|CONTEXT_EXTENDED_REGISTERS
#else
#error Unsupported CPU type
#endif
