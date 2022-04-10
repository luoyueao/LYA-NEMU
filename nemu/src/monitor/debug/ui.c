#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

//从sdtin流中读取一行命令
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg == NULL) { //若参数为空，则执行一步
    cpu_exec(1);
    return 0;
  }
  else {            //若参数非空，则将参数转换为uint64_t类型，执行N步
    uint64_t N;
    sscanf(arg, "%llu", &N);
    cpu_exec(N);
    return 0;
  }
}

static int cmd_info(char *args) {
  char *cmd = strtok(NULL, " ");
  if(strcmp(cmd, "r") == 0) {
    for(int i = 0; i < 8; i++) {  //输出32位寄存器的值
      printf("%s:\t%8x\t%d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);  
    }
    for(int i = 0; i < 8; i++) {  //输出16位寄存器的值
      printf("%s:\t%8x\t%d\n", regsw[i], cpu.gpr[i]._16, cpu.gpr[i]._16);
    }
    for(int i = 0; i < 4; i++) {  //输出8位寄存器的值
      printf("%s:\t%8x\t%d\t\t", regsb[i + 4], cpu.gpr[i]._8[1], cpu.gpr[i]._8[1]);
      printf("%s:\t%8x\t%d\n", regsb[i], cpu.gpr[i]._8[0], cpu.gpr[i]._8[0]);
    }
  }
  else if(strcmp(cmd, "w") == 0) {
    list_watchpoint();            //调用list_watchpoint()函数输出监视点信息
  }
  else if(strcmp(cmd, "b") == 0) {
    list_breakpoint();            //调用list_breakpoint()函数输出监视点信息
  }
  return 0;
}

static int cmd_x(char *args) {
    int N;                        //读取次数
    vaddr_t EXPR;                 //表达式求值的结果
    bool success = true;          //表达式求值是否成功
    char *cmd = strtok(NULL, " ");
    sscanf(cmd, "%d", &N);
    cmd = cmd + strlen(cmd) + 1;  //cmd指向表达式的起点
    EXPR = expr(cmd, &success);   //利用expr()函数求表达式的值
    if(!success) {
      printf("make_token() wrong!\n");
      return 0;
    }
    printf("Address    Dword block ... Byte sequence\n");
    for(int i = 0; i < N; i++) {
        printf("%#x   ", EXPR);   //输出读取起始地址
        printf("%#x\t   ", vaddr_read(EXPR, 4)); //输出起始地址往后4个字节的数据
        for(int j = 0; j < 4; j++) {
         printf("%x ", vaddr_read(EXPR, 1));     //输出起始地址往后1个字节的数据
         EXPR += 1;               //起始地址移1个字节
        }
        printf("\n");
    }
    return 0;
}

static int cmd_p(char *args) {
  bool success = true;
  int ans = expr(args, &success); //调用函数expr()对表达式args求值，返回结果
  if(!success) {                  //success标志位检查make_token()函数是否正常
    printf("make_token() wrong!\n");
  }
  else {
    printf("%d\n", ans);          //输出结果
  }
  return 0;
}

static int cmd_w(char *args) {
  if(args == NULL) {              //若参数为空，则输出提示
    printf("Argument required!\n");
  }
  else {                          //若参数非空，先对表达式求值，再添加新的监视点
    bool success = true;
    int ans = expr(args, &success);
    if(!success) {                //若make_token()出现错误，则输出提示
      printf("expression make_token() wrong!\n");
    }
    else {                        //设置新的监视点，并格式输出信息
      int NO = set_watchpoint(args);
      printf("Set watchpoint #%d\n", NO);
      printf("expr      = %s\n", args);
      printf("old value = %#x\n", ans);
    }
  }
  return 0;
}

static int cmd_d(char *args) {
  if(args == NULL) {              //若参数为空，则输出提示
    printf("Argument required!\n");
  }
  else {                          //若参数非空，从参数中取出编号NO
    int NO;
    sscanf(args, "%d", &NO);
    bool success;                 //判断delete_watchpoint()函数运行情况
    success = delete_watchpoint(NO);
    if(success)                   //输出成功信息
      printf("Watchpoint/Breakpoint %d deleted\n", NO);
    else                          //输出失败信息
      printf("Watchpoint/Breakpoint %d is not found\n", NO);
  }
  return 0;
}

static int cmd_b(char *args) {
  if(args == NULL) {              //若参数为空，则输出提示
    printf("Argument required!\n");
  }
  else {                          //若参数非空，将参数转换后传入set_breakpoint()，添加断点
    int NO;
    vaddr_t addr;
    sscanf(args, "%x", &addr);
    NO = set_breakpoint(addr);    //函数返回断点编号，下面输出信息
    printf("Breakpoint %d at %#x\n", NO, addr);
  }
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  { "si", "Implement single step / specified step execution function", cmd_si},
  { "info", "Output the value of all registers", cmd_info},
  { "x", "Output N consecutive 4 bytes in hexadecimal", cmd_x},
  { "p", "Calculate expression", cmd_p},
  { "w", "Set up a new monitoring point", cmd_w},
  { "d", "Delete the monitoring point", cmd_d},
  { "b", "Set a breakpoint", cmd_b}

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  //读取命令
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    //无参数的情况
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    //第一个token是命令
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    //剩余token作为参数
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
