#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // _switch(&pcb[i].as);
  // current = &pcb[i];
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  // 保存当前指针
  current->tf = prev;

  static int times = 0;                     //记录仙剑运行次数
  if(times == 200 && current == &pcb[0]) {  //若达到200次并正在运行仙剑
    times = 0;                              //次数清零
    current = &pcb[1];                      //转换到hello
  }
  else {
    times ++;                               //次数+1
    current = &pcb[0];                      //继续运行仙剑
  }

  // 切换到新程序空间
  _switch(&current->as);
  return current->tf;
}
