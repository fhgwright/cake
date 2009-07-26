#define DEBUG 1

#include <aros/system.h>

#define _GNU_SOURCE 1

#include <stddef.h>
#include <stdio.h>
#include <ucontext.h>

#include <exec/lists.h>
#include <exec/execbase.h>
#include <hardware/intbits.h>

#include "etask.h"
#include "kernel_intern.h"
#include "host_debug.h"

/* We have to redefine these flags here because including exec_intern.h causes conflicts
   between dos.h and WinAPI headers. This needs to be fixed - Pavel Fedin <sonic_amiga@rambler.ru */
#define SFB_SoftInt         5   /* There is a software interrupt */
#define SFF_SoftInt         (1L<<5)

#define ARB_AttnSwitch      7   /* Delayed Switch() pending */
#define ARF_AttnSwitch      (1L<<7)
#define ARB_AttnDispatch   15   /* Delayed Dispatch() pending */
#define ARF_AttnDispatch    (1L<<15)

/* We also have to define needed exec functions here because proto/exec.h also conflicts with
   WinAPI headers. */
#define Exception() AROS_LC0NR(void, Exception, struct ExecBase *, SysBase, 11, Exec)
#define Enqueue(arg1, arg2) AROS_LC2NR(void, Enqueue, \
				       AROS_LCA(struct List *,(arg1),A0), \
			               AROS_LCA(struct Node *,(arg2),A1), \
				       struct ExecBase *, SysBase, 45, Exec)

/*
 * Be careful with this, actually enabling this causes AROS to abort on first exception
 * because of OutputDebugString() calls. Looks like WinAPI functions love to perform stack
 * check and silently abort if they think something is wrong.
 */
#define DINT(x) D(x)
#define DS(x) D(x)
#define DSLEEP(x) D(x)

#define RETURN_FROM_INTERRUPT() do {    \
    if (SysBase->IDNestCnt < 0)         \
        core_intr_enable();             \
    return;                             \
} while (0)

/*
 * Task dispatcher. Basically it may be the same one no matter what scheduling algorithm is used
 */
void core_Dispatch(void)
{
    struct ExecBase *SysBase = *SysBasePtr;
    struct Task *task;

    D(bug("[kernel:scheduler] in core_Dispatch()\n"));

    core_intr_disable();

    /* 
     * Is the list of ready tasks empty? Well, increment the idle switch cound and stop the main thread.
     */
    if (IsListEmpty(&SysBase->TaskReady))
    {
        if (sleep_state != ss_RUNNING) {
            SysBase->IdleCount++;
            SysBase->AttnResched |= ARF_AttnSwitch;

            DSLEEP(bug("[kernel:scheduler] TaskReady list empty. Sleeping for a while...\n"));

            /* We are entering sleep mode */
	    sleep_state = ss_SLEEP_PENDING;
        }

        RETURN_FROM_INTERRUPT();
    }

    sleep_state = ss_RUNNING;
    SysBase->DispCount++;
        
    /* Get the first task from the TaskReady list, and populate it's settings through Sysbase */
    task = (struct Task *)REMHEAD(&SysBase->TaskReady);
    SysBase->ThisTask = task;  
    SysBase->Elapsed = SysBase->Quantum;
    SysBase->SysFlags &= ~0x2000;
    task->tc_State = TS_RUN;
    SysBase->IDNestCnt = task->tc_IDNestCnt;

    DS(bug("[kernel:scheduler] New task = %p (%s)\n", task, task->tc_Node.ln_Name));

    /* Handle tasks's flags */
    if (task->tc_Flags & TF_EXCEPT)
        Exception();
        
    if (task->tc_Flags & TF_LAUNCH)
    {
        task->tc_Launch(SysBase);       
    }
        
    /* Leave interrupt and jump to the new task */
    RETURN_FROM_INTERRUPT();
}

void core_Switch(void)
{
    struct ExecBase *SysBase = *SysBasePtr;
    struct Task *task;
    
    D(bug("[kernel:scheduler] in core_Switch()\n"));
    
    /* disable interrupts */
    core_intr_disable();

    task = SysBase->ThisTask;
        
    DS(bug("[kernel:scheduler] switching out task 0x%08lx %d %s\n", task, task->tc_Node.ln_Pri, task->tc_Node.ln_Name));
        
    /* store IDNestCnt into tasks's structure */  
    task->tc_IDNestCnt = SysBase->IDNestCnt;
    task->tc_SPReg = (APTR) get_stack_pointer((void *) GetIntETask(task)->iet_Context);

    /* And enable interrupts */
    SysBase->IDNestCnt = -1;
    core_intr_enable();
        
    /* TF_SWITCH flag set? Call the switch routine */
    if (task->tc_Flags & TF_SWITCH)
    {
        task->tc_Switch(SysBase);
    }
    
    core_Dispatch();
}


/*
 * Schedule the currently running task away. Put it into the TaskReady list 
 * in some smart way. This function is subject of change and it will be probably replaced
 * by some plugin system in the future
 */
void core_Schedule(void)
{
    struct ExecBase *SysBase = *SysBasePtr;
    struct Task *task;

    D(bug("[kernel:scheduler] in core_Schedule()\n"));

    /* disable interrupts */
    core_intr_disable();
            
    task = SysBase->ThisTask;
    
    /* Clear the pending switch flag. */
    SysBase->AttnResched &= ~ARF_AttnSwitch;
    
    /* If task has pending exception, reschedule it so that the dispatcher may handle the exception */
    if (!(task->tc_Flags & TF_EXCEPT))
    {
        /* Is the TaskReady empty? If yes, then the running task is the only one. Let it work */
        if (IsListEmpty(&SysBase->TaskReady))
            RETURN_FROM_INTERRUPT();
    
        /* Does the TaskReady list contains tasks with priority equal or lower than current task?
         * If so, then check further... */
        if (((struct Task*)GetHead(&SysBase->TaskReady))->tc_Node.ln_Pri <= task->tc_Node.ln_Pri)
        {
            /* If the running task did not used it's whole quantum yet, let it work */
            if (!(SysBase->SysFlags & 0x2000))
                RETURN_FROM_INTERRUPT();
        }
    }
    
    /* 
     * If we got here, then the rescheduling is necessary. 
     * Put the task into the TaskReady list.
     */
    task->tc_State = TS_READY;
    Enqueue(&SysBase->TaskReady, (struct Node *)task);
    
    /* Select new task to run */
    core_Switch();
}


/*
 * Leave the interrupt. This function receives the register frame used to leave the supervisor
 * mode. It reschedules the task if it was asked for.
 */
void core_ExitInterrupt(void)
{
    struct ExecBase *SysBase = *SysBasePtr;
    char TDNestCnt;

    D(bug("[kernel:scheduler] in core_ExitInterrupt()\n"));

#if DEBUG
    struct Task *task;

    bug("    SysBase 0x%08x\n", SysBase);

    bug("    current task: 0x%08x %d %s\n", SysBase->ThisTask, SysBase->ThisTask->tc_Node.ln_Pri, SysBase->ThisTask->tc_Node.ln_Name);
    
    bug("    ready tasks:\n");
    for (task = (struct Task *) SysBase->TaskReady.lh_Head; task->tc_Node.ln_Succ != NULL; task = (struct Task *) task->tc_Node.ln_Succ)
        bug("        0x%08x %d %s\n", task, task->tc_Node.ln_Pri, task->tc_Node.ln_Name);

    bug("    waiting tasks:\n");
    for (task = (struct Task *) SysBase->TaskWait.lh_Head; task->tc_Node.ln_Succ != NULL; task = (struct Task *) task->tc_Node.ln_Succ)
        bug("        0x%08x %d %s\n", task, task->tc_Node.ln_Pri, task->tc_Node.ln_Name);
#endif

    if (SysBase)
    {
        /* Soft interrupt requested? It's high time to do it */
        if (SysBase->SysFlags & SFF_SoftInt) {
            DS(bug("[kernel:scheduler] Causing SoftInt\n"));
            core_Cause(SysBase);
        }
    
        if (sleep_state != ss_RUNNING) {
            core_Dispatch();
            return;
        }
    
        /* If task switching is disabled, leave immediatelly */
        TDNestCnt = SysBase->TDNestCnt; /* BYTE is unsigned in Windows so we can't use SysBase->TDNestCnt directly */
        DS(bug("[kernel:scheduler] TDNestCnt is %d\n", TDNestCnt));
        if (TDNestCnt < 0)
        {
            /* 
             * Do not disturb task if it's not necessary. 
             * Reschedule only if switch pending flag is set. Exit otherwise.
             */
            if (SysBase->AttnResched & ARF_AttnSwitch)
            {
                DS(bug("[kernel:scheduler] Rescheduling\n"));
                core_Schedule();
            }
            else
                RETURN_FROM_INTERRUPT();
        }
    }
    	DS(else printf("[kernel:scheduler] SysBase is NULL\n");)
}

void core_Cause(struct ExecBase *SysBase)
{
    struct IntVector *iv = &SysBase->IntVects[INTB_SOFTINT];

    D(bug("[kernel:scheduler] in core_Cause()\n"));

    /* If the SoftInt vector in SysBase is set, call it. It will do the rest for us */
    if (iv->iv_Code)
    {
        iv->iv_Code(0, 0, 0, iv->iv_Code, SysBase);
    }
}
