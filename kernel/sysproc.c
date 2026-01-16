#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sysinfo(void)
{
  struct sysinfo info; // Tạo struct trong kernel stack
  uint64 addr; // Địa chỉ con trỏ user truyền vào

  // Lấy tham số đầu tiên (địa chỉ struct sysinfo từ user)
  if(argaddr(0, &addr) < 0)
    return -1;

  // Gọi 2 hàm helper đã viết ở Bước 1
  info.freemem = kcollect_free();
  info.nproc = pcollect_active();

  // Copy struct info từ kernel ra địa chỉ addr của user
  // copyout(pagetable, dst_va, src_pa, len)
  if(copyout(myproc()->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;

  return 0; 
}
// Trả về 0 nghĩa là thành công
uint64
sys_trace(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  myproc()->trace_mask = n;
  return 0;
}

int
sys_pgaccess(void)
{
  uint64 base;
  int len;
  uint64 mask_addr; // Địa chỉ buffer user
  struct proc *p = myproc();
  
  // Biến lưu kết quả tạm trong kernel (64 bits theo hướng dẫn)
  uint64 bitmask = 0; 

  // 1. Lấy 3 tham số: base, len, mask_addr
  if(argaddr(0, &base) < 0 || argint(1, &len) < 0 || argaddr(2, &mask_addr) < 0)
    return -1;

  // Giới hạn max 64 trang (vì bitmask là uint64)
  if(len > 64 || len < 0)
    return -1;

  // 2. Duyệt qua từng page
  for(int i = 0; i < len; i++){
    uint64 va = base + i * PGSIZE;
    
    // Tìm PTE bằng hàm walk
    pte_t *pte = walk(p->pagetable, va, 0);

    // Kiểm tra Valid và Access Bit
    if(pte &&
      (*pte & PTE_V) &&
      (*pte & PTE_U) &&  
      (*pte & PTE_A)){
        
      // Set bit tương ứng trong bitmask
      bitmask |= (1L << i);

      // Clear bit A (quan trọng để detect lần sau)
      *pte &= ~PTE_A; 
    }
  }

  // 3. Copy kết quả về user space
  // Lưu ý: Copy đúng 8 bytes (sizeof uint64)
  if(copyout(p->pagetable, mask_addr, (char *)&bitmask, sizeof(bitmask)) < 0)
    return -1;

  return 0;
}