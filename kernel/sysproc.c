#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

// 打印系统调用信息
uint64
sys_trace(void)
{
  int mask;
  // 从a0寄存器中获取追踪的mask
  if(argint(0, &mask) < 0)
    return -1;
  // 将mask保存在本进程的PCB中
  myproc()->trace_mask = mask;
  return 0;
}


// 收集xv6运行的一些信息：当前剩余的内存字节数；状态为UNUSED 的进程个数；当前进程可用文件描述符的数量
uint64
sys_sysinfo(void)
{
  uint64 addr;
  struct sysinfo info;
  // 从a0寄存器中获取用户态地址addr，该地址指向struct sysinfo
  if(argaddr(0, &addr) < 0)
    return -1;
  // 获取数据
  info.freemem = get_freemem();
  info.nproc   = get_nproc();
  info.freefd  = get_freefd();
  // 将内核态地址中填写的结构体复制到用户态地址
  if(copyout(myproc()->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}
