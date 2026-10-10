#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "procinfo.h"


extern struct proc proc[NPROC];
extern struct spinlock wait_lock;

uint64 sys_ps_listinfo(void) {
    uint64 plist;
    int lim;

    argaddr(0, &plist);
    argint(1, &lim);

    if ((plist == 0) || lim < 0) {
        return -1;
    }

    struct proc* currentProc;
    struct proc* srcProc = myproc();

    int addedInList = 0;
    int totalProc = 0;


    for (int i = 0; i < NPROC; i++) {
        currentProc = &proc[i];

        struct procinfo procinfo;

        acquire(&currentProc->lock);

        procinfo.pid = currentProc->pid;
        procinfo.state = currentProc->state;

        release(&currentProc->lock);

        if ((procinfo.state == PS_UNUSED) || (procinfo.state == PS_USED)) {
            continue;
        }

        totalProc++;

        // написано, что name можно без блокировки
        for (int index = 0; index < 16; index++) {
            procinfo.name[index] = (currentProc->name)[index];
        }

        procinfo.ppid = 0;
        acquire(&wait_lock);
        struct proc *parent = currentProc->parent;

        if (parent != 0) {
            acquire(&parent->lock);

            procinfo.ppid = parent->pid;
            
            release(&parent->lock);
        }

        release(&wait_lock);

        if (addedInList >= lim) {
            continue;
        }
        
        int res = copyout(
            srcProc->pagetable,
            srcProc->sz, plist + addedInList * sizeof(struct procinfo),
            (char *) &procinfo, 
            sizeof(struct procinfo)
        );

        if (res == -1) {
            return -1;
        }

        addedInList++;
    }

    return totalProc;
}