#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

void
pgaccess_test()
{
  char *buf;
  uint64 abits;
  printf("pgaccess_test: starting\n");

  // test that pgaccess fails with bad arguments
  if(pgaccess(0, 0, 0) != -1){
    printf("pgaccess_test: failed (1)\n");
    exit(1);
  }

  // allocate a page
  buf = malloc(32 * PGSIZE);
  if (buf == 0) {
    printf("pgaccess_test: cannot malloc\n");
    exit(1);
  }

  // read the buffer, which hits the pages
  // access page 1, 2, 30
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;

  // call pgaccess to report which pages were accessed
  if (pgaccess(buf, 32, &abits) < 0) {
    printf("pgaccess_test: pgaccess failed\n");
    exit(1);
  }

  // check the result
  if (abits != ((1 << 1) | (1 << 2) | (1 << 30))) {
    printf("pgaccess_test: incorrect access bits: %lx\n", abits);
    printf("expected: %lx\n", (uint64)((1 << 1) | (1 << 2) | (1 << 30)));
    exit(1);
  }

  printf("pgaccess_test: OK\n");
}

int
main(int argc, char *argv[])
{
  pgaccess_test();
  exit(0);
}