#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define NPAGE 8

// static buffer để tránh malloc/sbrk
static char buf[NPAGE * PGSIZE];

int
main(void)
{
  uint64 mask;

  printf("TEST 1: no access yet\n");
  mask = 0;
  if (pgaccess(buf, NPAGE, &mask) < 0) {
    printf("pgaccess failed\n");
    exit(1);
  }
  printf("mask = 0x%lx (expected 0x0)\n\n", mask);

  printf("TEST 2: touch page 0\n");
  buf[0] = 'A';
  mask = 0;
  pgaccess(buf, NPAGE, &mask);
  printf("mask = 0x%lx (expected 0x1)\n\n", mask);

  printf("TEST 3: call again without touching\n");
  mask = 0;
  pgaccess(buf, NPAGE, &mask);
  printf("mask = 0x%lx (expected 0x0)\n\n", mask);

  printf("TEST 4: touch pages 1, 3, 6\n");
  buf[1 * PGSIZE] = 'B';
  buf[3 * PGSIZE] = 'C';
  buf[6 * PGSIZE] = 'D';

  mask = 0;
  pgaccess(buf, NPAGE, &mask);
  printf("mask = 0x%lx (expected 0x4a)\n", mask);
  // 0x4a = 0b01001010 = pages 1,3,6

  printf("\nALL TESTS DONE\n");
  exit(0);
}
