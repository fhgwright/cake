/* theory of operation
 *
 * at the start, there's a single process. core_init() is called system
 * initialisation. it starts a thread, the switcher thread.
 *
 * the main thread runs the "current" aros task. thats all. if left unchecked,
 * it would run the same task forever. thats not much fun though, which is
 * what the switcher thread is for.
 *
 * the switcher thread receives interrupts from a variety of sources (virtual
 * hardware) including a timer thread (for normal task switch operation). when
 * an interrupt occurs, the following happens (high level)
 *
 * - switcher signals main that its time for a context switch
 * - main responds by storing the current context and then waiting
 * - switcher saves the current context into the current aros task
 * - switcher runs the scheduler with the current interrupt state
 * - switcher loads the context for the scheduled task as the new current context
 * - switcher signals main
 * - main wakes up and jumps to the current context
 * - switcher loops and waits for the next interrupt
 */

#define DEBUG 1

#include <aros/system.h>
#include <exec/types.h>

#include <stddef.h>
#include <stdio.h>
#include <ucontext.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include <exec/lists.h>
#include <exec/execbase.h>
#include "kernel_intern.h"
#include "syscall.h"
#include "host_debug.h"
#include "cpucontext.h"

struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

int irq_enabled;
int in_supervisor;
int sleep_state;

void core_intr_disable(void) {
    D(bug("[kernel] disabling interrupts\n"));
    irq_enabled = 0;
}

void core_intr_enable(void) {
    int i;

    D(bug("[kernel] enabling interrupts\n"));
    irq_enabled = 1;
}

int core_in_supervisor(void) {
    return in_supervisor;
}

pthread_t main_thread;
pthread_t switcher_thread;
pthread_t timer_thread;

unsigned long timer_period;

pthread_mutex_t irq_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t irq_cond = PTHREAD_COND_INITIALIZER;
uint32_t irq_bits;

sem_t main_sem;
sem_t switcher_sem;

ucontext_t main_ctx;

static void *timer_entry(void *arg) {
    struct timespec ts;

    while (1) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 1000000000 / timer_period;
        if (ts.tv_nsec > 999999999) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);

        pthread_mutex_lock(&irq_lock);
        irq_bits |= 0x1;
        pthread_mutex_unlock(&irq_lock);

        pthread_cond_signal(&irq_cond);
    }
}

static void *switcher_entry(void *arg) {
    irq_bits = 0;

    sem_init(&main_sem, 0, 0);
    sem_init(&switcher_sem, 0, 0);

    while (1) {
        /* wait for an interrupt */
        pthread_mutex_lock(&irq_lock);
        pthread_cond_wait(&irq_cond, &irq_lock);

        D(bug("[kernel] interrupt received, irq bits are 0x%x\n", irq_bits));

        /* tell the main task to stop and wait for its signal to proceed */
        sem_post(&main_sem);
        pthread_kill(main_thread, SIGUSR1);
        sem_wait(&switcher_sem);

        /* allow new interrupts */
        irq_bits = 0;
        pthread_mutex_unlock(&irq_lock);

        /* run the scheduler */
        core_ExitInterrupt(&main_ctx);

        /* tell the main task to switch the context */
        sem_post(&main_sem);
    }

    return NULL;
}

static void main_switch_handler(int signo, siginfo_t *si, void *vctx) {
    /* make sure we were signalled by the switcher thread and not somewhere else */
    if (sem_trywait(&main_sem) < 0)
        return;

    /* switcher thread is now waiting for us. save the current context somewhere it can get it */
    memcpy(&main_ctx, vctx, sizeof(ucontext_t));

    /* tell the switcher to proceed */
    sem_post(&switcher_sem);

    /* wait for it to run the scheduler and whatever else */
    sem_wait(&main_sem);

    /* switcher has given us the new context, jump to it */
    setcontext(&main_ctx);
}

int core_init(unsigned long TimerPeriod, struct ExecBase **SysBasePointer, struct KernelBase **KernelBasePointer) {
    struct sigaction sa;
    pthread_attr_t thread_attrs;

    D(bug("[kernel] initialising interrupts and task switching\n"));

    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;

    irq_enabled = 0;
    in_supervisor = 0;
    sleep_state = 0;

    timer_period = TimerPeriod;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = main_switch_handler;
    sigaction(SIGUSR1, &sa, NULL);

    main_thread = pthread_self();

    pthread_attr_init(&thread_attrs);
    pthread_attr_setattr(&thread_attrs, PTHREAD_CREATE_DETACHED);
    pthread_create(&switcher_thread, &thread_attrs, switcher_entry, NULL);
    pthread_create(&timer_thread, &thread_attrs, timer_entry, NULL);

    D(bug("[kernel] threads started, switcher id %d, timer id %d\n", switcher_thread, timer_thread));

    return 0;
}
