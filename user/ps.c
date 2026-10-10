#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void printPlist(struct procinfo* plist, int size);

int main(int argc, char** argv) {
    int size = 1;
    int procCount = 0;
    struct procinfo* plist = 0;

    int oldSize = -1;

    while (oldSize < procCount) {
        plist = malloc(sizeof(struct procinfo) * size);
        if (plist == 0) {
            return -1;
        }
        

        procCount = ps_listinfo(plist, size);

        printf("procCount: %d\n", procCount);
        printPlist(plist, size);

        free(plist);
        plist = 0;

        oldSize = size;
        size *= 2;
    }

    return 0;
}

void printPlist(struct procinfo* plist, int size) {
    if (plist == 0) {
        return;
    }
    
    printf("%s", "pid\tname\tstate\t\tppid\n");
    printf("%s", "-------------------------------------\n");

    for (int i = 0; i < size; i++) {
        struct procinfo* currentProcinfo = &plist[i];

        if (currentProcinfo->pid == 0) {
            break;
        }

        printf("%d\t%s\t", currentProcinfo->pid, currentProcinfo->name);

        switch (currentProcinfo->state) {
        case PS_UNUSED:
            break;

        case PS_USED:
            break;

        case PS_SLEEPING:
            printf("%s", "SLEEPING\t");
            break;
        
        case PS_RUNNABLE:
            printf("%s", "RUNNABLE\t");
            break;
        
        case PS_RUNNING:
            printf("%s", "RUNNING\t\t");
            break;
        
        case PS_ZOMBIE:
            printf("%s", "ZOMBIE\t");
            break;
        }
        
        printf("%d\n", currentProcinfo->ppid);
    
    }   
}