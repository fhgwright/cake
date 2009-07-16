#define DEBUG 1

#include <aros/system.h>
#include <exec/types.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include <exec/lists.h>
#include <exec/execbase.h>

#include "etask.h"

#include "kernel_intern.h"
#include "syscall.h"
#include "host_debug.h"

#define IRQ_TIMER   (1<<0)
#define IRQ_SYSCALL (1<<1)

struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

static int irq_enabled;
static int in_supervisor;
int sleep_state;

void core_intr_disable(void) {
    //D(printf("[kernel] disabling interrupts\n"));
    irq_enabled = 0;
}

void core_intr_enable(void) {
    //D(printf("[kernel] enabling interrupts\n"));
    irq_enabled = 1;
}

int core_is_super(void) {
    return in_supervisor;
}

static pid_t pid;

static ucontext_t irq_ctx;
static void *irq_stack;

static unsigned long timer_period;

static ucontext_t idle_ctx;

void core_syscall(syscall_id_t type) {
    sigqueue(pid, SIGUSR1, (const union sigval) (int) (IRQ_SYSCALL | ((type & 0xffff) << 16)));
}

static void *timer_entry(void *arg) {
    sigset_t sigset;
    struct timespec ts;

    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    while (1) {
        D(printf("[kernel:timer] sleeping\n"));

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 1000000000 / timer_period;
        if (ts.tv_nsec > 999999999) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);

        D(printf("[kernel:timer] timer expiry, triggering timer interrupt\n"));

        sigqueue(pid, SIGUSR1, (const union sigval) IRQ_TIMER);
    }
}

static void irq_handler (int irq_bits) {
    in_supervisor++;

    if (irq_bits & IRQ_SYSCALL) {
        switch ((irq_bits & 0xffff0000) >> 16) {
            case sc_CAUSE:
                core_Cause(*SysBasePtr);
                break;

            case sc_DISPATCH:
                core_Dispatch();
                break;

            case sc_SWITCH:
                core_Switch();
                break;

            case sc_SCHEDULE:
                core_Schedule();
                break;
        }
    }

    if (irq_enabled)
        core_ExitInterrupt();

    in_supervisor--;

    if (sleep_state != ss_RUNNING) {
        sleep_state = ss_SLEEPING;

        setcontext(&idle_ctx);
    }

    setcontext((ucontext_t *) GetIntETask((*SysBasePtr)->ThisTask)->iet_Context);
}

static void irq_trampoline (int signo, siginfo_t *si, void *vctx) {
    if (!irq_enabled && !(si->si_value.sival_int & IRQ_SYSCALL)) return;

    getcontext(&irq_ctx);
    irq_ctx.uc_stack.ss_sp = irq_stack;
    irq_ctx.uc_stack.ss_size = SIGSTKSZ;
    irq_ctx.uc_stack.ss_flags = 0;
    sigemptyset(&irq_ctx.uc_sigmask);
    makecontext(&irq_ctx, (void (*)()) irq_handler, 1, si->si_value.sival_int);

    swapcontext((ucontext_t *) GetIntETask((*SysBasePtr)->ThisTask)->iet_Context, &irq_ctx);
}

static void idle_handler (void) {
    while (1) sleep(INT_MAX);
}

int core_init (unsigned long TimerPeriod, struct ExecBase **SysBasePointer, struct KernelBase **KernelBasePointer) {
    sigset_t sigset;
    struct sigaction sa;
    pthread_t thread;
    pthread_attr_t thread_attrs;

    D(printf("[kernel] initialising interrupts and task switching\n"));

    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;

    irq_enabled = 0;
    in_supervisor = 0;
    sleep_state = 0;

    timer_period = TimerPeriod;

    irq_stack = malloc(SIGSTKSZ);

    pid = getpid();

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = irq_trampoline;
    sigaction(SIGUSR1, &sa, NULL);

    getcontext(&idle_ctx);
    idle_ctx.uc_stack.ss_sp = malloc(SIGSTKSZ);
    idle_ctx.uc_stack.ss_size = SIGSTKSZ;
    idle_ctx.uc_stack.ss_flags = 0;
    makecontext(&idle_ctx, (void (*)()) idle_handler, 0);

    pthread_attr_init(&thread_attrs);
    pthread_attr_setdetachstate(&thread_attrs, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &thread_attrs, timer_entry, NULL);

    D(printf("[kernel] timer started, frequency is %dhz\n", timer_period));

    return 0;
}
