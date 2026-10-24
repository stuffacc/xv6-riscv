#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  void *a;
  void *b;
  void *c;

  a = malloc(20);
  b = malloc(100);
  c = malloc(500);

  free(a);
  free(b);
  free(c);

  exit(0);
}
