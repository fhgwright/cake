#define _GNU_SOURCE 1
#include <ucontext.h>

#ifdef __i386__

#define PRINT_CPUCONTEXT(ctx) \
    kprintf ("    ESP=%08lx  EBP=%08lx  EIP=%08lx\n" \
             "    EAX=%08lx  EBX=%08lx  ECX=%08lx  EDX=%08lx\n" \
             "    EDI=%08lx  ESI=%08lx  EFLAGS=%08lx\n" \
             , ctx->uc_mcontext.gregs[REG_ESP] \
             , ctx->uc_mcontext.gregs[REG_EBP] \
             , ctx->uc_mcontext.gregs[REG_EIP] \
             , ctx->uc_mcontext.gregs[REG_EAX] \
             , ctx->uc_mcontext.gregs[REG_EBX] \
             , ctx->uc_mcontext.gregs[REG_ECX] \
             , ctx->uc_mcontext.gregs[REG_EDX] \
             , ctx->uc_mcontext.gregs[REG_EDI] \
             , ctx->uc_mcontext.gregs[REG_ESI] \
             , ctx->uc_mcontext.gregs[REG_EFL] \
    );

#define PREPARE_INITIAL_CONTEXT(ctx, sp, pc) \
    ctx->uc_mcontext.gregs[REG_EBP] = 0; \
    ctx->uc_mcontext.gregs[REG_EIP] = (IPTR) pc; \
    ctx->uc_mcontext.gregs[REG_ESP] = (IPTR) sp;

#else
#error Unsupported CPU type
#endif
