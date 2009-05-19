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
 * hardware) including its own timer (for normal task switch operation). when
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

#define DI(x)   /* Interrupts debug     */
#define DS(x)   /* Task switcher debug  */
#define DIRQ(x) /* IRQ debug		*/

#define AROS_EXCEPTION_SYSCALL 0x80000001

struct SwitcherData {
    HANDLE MainThread;
    HANDLE IntObjects[INTERRUPTS_NUM];
};

struct SwitcherData SwData;
DWORD *LastErrorPtr;
unsigned char Ints_Enabled;
unsigned char PendingInts[INTERRUPTS_NUM];
unsigned char Supervisor;
unsigned char Sleep_Mode;
struct ExecBase **SysBasePtr;
struct KernelBase **KernelBasePtr;

void user_handler(uint8_t exception, struct List *list)
{
    if (!IsListEmpty(&list[exception]))
    {
        struct IntrNode *in, *in2;

        ForeachNodeSafe(&list[exception], in, in2)
        {
            if (in->in_Handler)
                in->in_Handler(in->in_HandlerData, in->in_HandlerData2);
        }
    }
}

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS Except)
{
    	struct ExecBase *SysBase = *SysBasePtr;
    	struct KernelBase *KernelBase = *KernelBasePtr;
    	REG_SAVE_VAR;

	Supervisor++;
	switch (Except->ExceptionRecord->ExceptionCode) {
	case AROS_EXCEPTION_SYSCALL:
	    CONTEXT_SAVE_REGS(Except->ContextRecord);
	    DI(printf("[KRN] Syscall exception %lu\n", *Except->ExceptionRecord->ExceptionInformation));
	    switch (*Except->ExceptionRecord->ExceptionInformation)
	    {
	    case SC_CAUSE:
	        core_Cause(SysBase);
	        break;
	    case SC_DISPATCH:
	        core_Dispatch(Except->ContextRecord);
	        break;
	    case SC_SWITCH:
	        core_Switch(Except->ContextRecord);
	        break;
	    case SC_SCHEDULE:
	        core_Schedule(Except->ContextRecord);
	        break;
	    }
	    CONTEXT_RESTORE_REGS(Except->ContextRecord);
	    Supervisor--;
	    return EXCEPTION_CONTINUE_EXECUTION;
	default:
	    printf("[KRN] Exception 0x%08lX handler. Context @ %p, SysBase @ %p, KernelBase @ %p\n", Except->ExceptionRecord->ExceptionCode, Except->ContextRecord, SysBase, KernelBase);
    	    if (SysBase)
    	    {
        	struct Task *t = SysBase->ThisTask;
        	
        	if (t)
        	    printf("[KRN] %s %p (%s)\n", t->tc_Node.ln_Type == NT_TASK ? "Task":"Process", t, t->tc_Node.ln_Name ? t->tc_Node.ln_Name : "--unknown--");
        	else
        	    printf("[KRN] No task\n");
    	    }
    	    PRINT_CPUCONTEXT(Except->ContextRecord);
    	    printf("[KRN] **UNHANDLED EXCEPTION** stopping here...\n");
	    return EXCEPTION_EXECUTE_HANDLER;
	}
}

DWORD WINAPI TaskSwitcher(struct SwitcherData *args)
{
    HANDLE IntEvent;
    DWORD obj;
    CONTEXT MainCtx;
    REG_SAVE_VAR;
    DS(DWORD res);
    MSG msg;

    for (;;) {
        obj = WaitForMultipleObjects(INTERRUPTS_NUM, args->IntObjects, FALSE, INFINITE);
        DS(bug("[Task switcher] Object %lu signalled\n", obj));
        if (Sleep_Mode != SLEEP_MODE_ON) {
            DS(res =) SuspendThread(args->MainThread);
    	    DS(bug("[Task switcher] Suspend thread result: %lu\n", res));
    	}
        if (Ints_Enabled) {
    	    Supervisor++;
    	    PendingInts[obj] = 0;
    	    /* 
    	     * We will get and store the complete CPU context, but set only part of it.
    	     * This can be a useful aid for future AROS debuggers.
    	     */
    	    CONTEXT_INIT_FLAGS(&MainCtx);
    	    DS(res =) GetThreadContext(args->MainThread, &MainCtx);
    	    DS(bug("[Task switcher] Get context result: %lu\n", res));
    	    CONTEXT_SAVE_REGS(&MainCtx);
    	    DS(OutputDebugString("[Task switcher] original CPU context: ****\n"));
    	    DS(PrintCPUContext(&MainCtx));
    	    if (*KernelBasePtr)
	    	user_handler(obj, (*KernelBasePtr)->kb_Interrupts);
    	    core_ExitInterrupt(&MainCtx);
    	    if (!Sleep_Mode) {
    	        DS(OutputDebugString("[Task switcher] new CPU context: ****\n"));
    	        DS(PrintCPUContext(&MainCtx));
    	        CONTEXT_RESTORE_REGS(&MainCtx);
    	        DS(res =)SetThreadContext(args->MainThread, &MainCtx);
    	        DS(bug("[Task switcher] Set context result: %lu\n", res));
    	    }
    	    Supervisor--;
    	} else {
    	    PendingInts[obj] = 1;
            DS(bug("[KRN] Interrupts are disabled, interrupt %lu is pending\n", obj));
        }
        if (Sleep_Mode)
            /* We've entered sleep mode */
            Sleep_Mode = SLEEP_MODE_ON;
        else {
            DS(res =) ResumeThread(args->MainThread);
            DS(bug("[Task switcher] Resume thread result: %lu\n", res));
        }
    }
    return 0;
}

/* ****** Interface functions ****** */

long __declspec(dllexport) core_intr_disable(void)
{
    DI(printf("[KRN] disabling interrupts\n"));
    Ints_Enabled = 0;
}

long __declspec(dllexport) core_intr_enable(void)
{
    int i;

    DI(printf("[KRN] enabling interrupts\n"));
    Ints_Enabled = 1;
    /* FIXME: here we do not force timer interrupt, probably this is wrong. However there's no way
       to force-trigger a waitable timer in Windows. A workaround is possible, but the design will
       be complicated then (we need a companion event in this case). Probably it will be implemented
       in future. */
    for (i = INT_IO; i < INTERRUPTS_NUM; i++) {
        if (PendingInts[i]) {
            DI(printf("[KRN] enable: sigalling about pending interrupt %lu\n", i));
            SetEvent(SwData.IntObjects[i]);
        }
    }
}

void __declspec(dllexport) core_syscall(unsigned long n)
{
    RaiseException(AROS_EXCEPTION_SYSCALL, 0, 1, &n);
    /* If after RaiseException we are still here, but Sleep_Mode != 0, this likely means
       we've just called SC_SCHEDULE, SC_SWITCH or SC_DISPATCH, and it is putting us to sleep.
       Sleep mode will be committed as soon as timer IRQ happens */
    while(Sleep_Mode) {
    	/* TODO: SwitchToThread() here maybe? But it's dangerous because context switch
    	   will happen inside it and Windows will kill us */
    }
}

unsigned char __declspec(dllexport) core_is_super(void)
{
    return Supervisor;
}

BOOL InitIntObjects(HANDLE *Objs)
{
    int i;

    for (i = 0; i < INTERRUPTS_NUM; i++) {
        Objs[i] = NULL;
        PendingInts[i] = 0;
    }
    /* Timer interrupt is a waitable timer, it's not an event */
    for (i = INT_IO; i < INTERRUPTS_NUM; i++) {
        Objs[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!Objs[i])
            return FALSE;
    }
    return TRUE;
}

void CleanupIntObjects(HANDLE *Objs)
{
    int i;

    for (i = 0; i < INTERRUPTS_NUM; i++) {
        if (Objs[i])
            CloseHandle(Objs[i]);
    }
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

void *timer_entry(void *arg) {
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

void *switcher_entry(void *arg) {
    irq_bits = 0;

    sem_init(&main_sem, 0, 0);
    sem_init(&switcher_sem, 0, 0);

    while (1) {
        /* wait for an interrupt */
        pthread_mutex_lock(&irq_lock);
        pthread_cond_wait(&irq_cond, &irq_lock);

        /* tell the main task to stop and wait for its signal to proceed */
        sem_post(&main_sem);
        pthread_kill(main_thread, SIGUSR1);
        sem_wait(&switcher_sem);

        /* allow new interrupts */
        pthread_mutex_unlock(&irq_lock);

        /* run the scheduler */
        core_ExitInterrupt(&main_ctx);

        /* tell the main task to switch the context */
        sem_post(&main_sem);
    }

    return NULL;
}

void main_switch_handler(int signo, siginfo_t *si, void *vctx) {
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

    D(printf("[kernel] initialising interrupts and task switching\n"));

    SysBasePtr = SysBasePointer;
    KernelBasePtr = KernelBasePointer;

    Ints_Enabled = 0;
    Supervisor = 0;
    Sleep_Mode = 0;

    timer_period = TimerPeriod;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = main_switch_handler;
    sigaction(SIGUSR1, &sa, NULL);

    main_thread = pthread_self();

    pthread_attr_init(&thread_attrs);
    pthread_attr_setattr(&thread_attrs, PTHREAD_CREATE_DETACHED);
    pthread_create(&switcher_thread, &thread_attrs, switcher_entry, NULL);
    pthread_create(&timer_thread, &thread_attrs, timer_entry, NULL);

    D(printf("[kernel] threads started, switcher id %d, timer id %d\n", switcher_thread, timer_thread));

    return 0;
}

/*
 * This is the only function to be called by modules other than kernel.resource.
 * It is used for causing interrupts from within asynchronous threads of
 * emul.handler and wingdi.hidd.
 */

unsigned long __declspec(dllexport) KrnCauseIRQ(unsigned char irq)
{
    unsigned long res;

    D(printf("[kernel IRQ] Causing IRQ %u\n", irq));
    res = SetEvent(SwData.IntObjects[irq]);
    D(printf("[kernel IRQ] Result: %ld\n", res));
    return res;
}
