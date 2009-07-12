#define _GNU_SOURCE 1

#include <stdio.h>
#include <ucontext.h>

int core_get_context_size (void) {
    return sizeof(ucontext_t);
}

void core_print_context (void *vctx) {
    ucontext_t *ctx = (ucontext_t *) vctx;

    fprintf (stderr, "Register dump (context 0x%08lx):\n"
                     "    esp=%08lx  ebp=%08lx  eip=%08lx\n"
                     "    eax=%08lx  ebx=%08lx  ecx=%08lx  edx=%08lx\n"
                     "    edi=%08lx  esi=%08lx  efl=%08lx\n",
                     ctx,
                     ctx->uc_mcontext.gregs[REG_ESP],
                     ctx->uc_mcontext.gregs[REG_EBP],
                     ctx->uc_mcontext.gregs[REG_EIP],
                     ctx->uc_mcontext.gregs[REG_EAX],
                     ctx->uc_mcontext.gregs[REG_EBX],
                     ctx->uc_mcontext.gregs[REG_ECX],
                     ctx->uc_mcontext.gregs[REG_EDX],
                     ctx->uc_mcontext.gregs[REG_EDI],
                     ctx->uc_mcontext.gregs[REG_ESI],
                     ctx->uc_mcontext.gregs[REG_EFL]
    );
}

void core_prepare_context(void *vctx, void *sp, void *pc) {
    ucontext_t *ctx = (ucontext_t *) vctx;

    getcontext(ctx);

    ctx->uc_mcontext.gregs[REG_EBP] = 0;
    ctx->uc_mcontext.gregs[REG_EIP] = (greg_t) pc;
    ctx->uc_mcontext.gregs[REG_ESP] = (greg_t) sp;
}
