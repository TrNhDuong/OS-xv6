#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int main()
{
  struct sysinfo info;

  if(sysinfo(&info) < 0){
    printf("sysinfotest: failed\n");
    exit(1);
  }

  printf("free memory: %ld\n", info.freemem);
  printf("num proc:    %ld\n", info.nproc);

  printf("sysinfotest: OK\n");
  exit(0);
}