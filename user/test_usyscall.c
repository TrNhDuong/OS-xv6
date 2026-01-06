#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  int standard = getpid();  // Gọi qua kernel
  int optimized = ugetpid(); // Gọi qua shared memory

  printf("PID standard:  %d\n", standard);
  printf("PID optimized: %d\n", optimized);

  if (standard == optimized) {
    printf("TEST PASSED\n");
  } else {
    printf("TEST FAILED\n");
  }
  
  exit(0);
}