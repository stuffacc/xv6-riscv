#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define NBINS 9
#define ROUND8(n) (((n) + 7) & ~7)

typedef struct block Block;

struct block {
  uint size;        
  uint requested; 
  int free;
  Block *next;
};

static Block *bins[NBINS];
static char *heap_start;
static char *heap_end;
static int debug = 1;

static int
bin_number(uint size)
{
  if (size <= 16)
    return 0;
  if (size <= 32)
    return 1;
  if (size <= 64)
    return 2;
  if (size <= 128)
    return 3;
  if (size <= 256)
    return 4;
  if (size <= 512)
    return 5;
  if (size <= 1024)
    return 6;
  if (size <= 2048)
    return 7;
  return 8;
}

static void
add_free(Block *b)
{
  int n = bin_number(b->size);

  b->free = 1;
  b->requested = 0;
  b->next = bins[n];
  bins[n] = b;
}

static void
remove_free(Block *b)
{
  Block **p = &bins[bin_number(b->size)];

  while (*p && *p != b)
    p = &(*p)->next;

  if (*p == b)
    *p = b->next;

  b->next = 0;
}

static Block *
find_free(uint size)
{
  int i;
  Block *b;

  for (i = bin_number(size); i < NBINS; i++) {
    for (b = bins[i]; b; b = b->next) {
      if (b->size >= size)
        return b;
    }
  }

  return 0;
}

static void
print_heap(void)
{
  Block *b;
  uint requested = 0;
  uint allocated = 0;
  uint heap_size = 0;

  if (heap_start && heap_end)
    heap_size = (uint)(heap_end - heap_start);

  printf("\nheap end=%p\n", heap_end);

  for (b = (Block *)heap_start;
       b && (char *)b < heap_end;
       b = (Block *)((char *)b + b->size)) {
    printf("%p size=%u %s",
           b, b->size, b->free ? "free" : "used");

    if (!b->free) {
      requested += b->requested;
      allocated += b->size;
    }

    printf("\n");
  }

  printf("requested=%u allocated=%u heap=%u usage=%u%%\n",
         requested,
         allocated,
         heap_size,
         heap_size ? requested * 100 / heap_size : 0);
}

void
malloc_debug(int enabled)
{
  debug = enabled;
}

void *
malloc(uint nbytes)
{
  Block *b;
  Block *rest;
  uint need;
  uint pages;

  if (nbytes == 0)
    return 0;

  need = ROUND8(sizeof(Block) + nbytes);
  b = find_free(need);

  if (b) {
    remove_free(b);
  } else {
    pages = ((need + PGSIZE - 1) / PGSIZE) * PGSIZE;
    b = (Block *)sbrk(pages);

    if ((char *)b == SBRK_ERROR)
      return 0;

    if (!heap_start)
      heap_start = (char *)b;

    heap_end = (char *)b + pages;
    b->size = pages;
    b->requested = 0;
    b->free = 0;
    b->next = 0;
  }

  if (b->size >= need + sizeof(Block) + 8) {
    rest = (Block *)((char *)b + need);
    rest->size = b->size - need;
    rest->requested = 0;
    rest->next = 0;

    b->size = need;
    add_free(rest);
  }

  b->free = 0;
  b->requested = nbytes;
  b->next = 0;

  if (debug)
    print_heap();

  return (void *)(b + 1);
}

void
free(void *ptr)
{
  Block *b;

  if (!ptr)
    return;

  b = (Block *)ptr - 1;

  add_free(b);

  if (debug)
    print_heap();
}
