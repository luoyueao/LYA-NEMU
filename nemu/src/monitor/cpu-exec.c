#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <time.h>

long loop_count_start;  //起始时间

//最大执行指令数
#define MAX_INSTR_TO_PRINT 1000001

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

//模拟取指-译码-执行的过程
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

#ifdef CATCH_LOOP
  loop_count_start = clock();
#endif

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  scan_breakpoint();                            //将设置的断点的指令首字节替换

  for (; n > 0; n --) {
    
    exec_wrapper(print_flag);

  if(check_breakpoint()) {                       //若触发断点，则暂停程序
    nemu_state = NEMU_STOP;
  }

#ifdef DEBUG
    WP *p;
    p = scan_watchpoint();                      //扫描监视点
    if(p) {                                     //若触发监视点
      nemu_state = NEMU_STOP;                   //暂停
      printf("Hit watchpoint %d at address %#x\n", p->NO, expr("$eip", NULL));
      printf("expr      = %s\n", p->expr);
      printf("old value = %#x\n", p->old_val);
      printf("new value = %#x\n", p->new_val);
      printf("program paused\n");
      p->old_val = p->new_val;                  //输出完信息后将旧值用新值替换
    }
#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
