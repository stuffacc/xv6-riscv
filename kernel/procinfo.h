enum state { PS_UNUSED, PS_USED, PS_SLEEPING, PS_RUNNABLE, PS_RUNNING, PS_ZOMBIE };

struct procinfo {
  int pid;
  char name[16];
  enum state state;
  int ppid;
};
