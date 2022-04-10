#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr[100];   //存储被监视的表达式
  int new_val;      //表达式的新值，当新值和旧值不匹配时，触发程序暂停运行
  int old_val;      //表达式的旧值

  bool type;        //类型，0表示监视点，1表示断点
  vaddr_t addr;     //断点地址字段
  uint32_t sub;     //被替换字节内容
  bool activation;  //断点是否刚刚被触发
} WP;

int set_watchpoint(char *e);    //给予一个表达式e，构造以该表达式为监视目标的监视点，并返回编号
bool delete_watchpoint(int NO); //给予一个监视点编号，从已使用的监视点中归还该监视点到池中
void list_watchpoint();         //显示当前在使用状态中的监视点列表
WP* scan_watchpoint();          //扫描所有使用中的监视点，返回触发的监视点指针，若无触发返回NULL

int set_breakpoint(vaddr_t addr);  //给予一个地址addr，构造以该地址为目标的断点，并返回编号
void list_breakpoint();             //显示当前在使用状态中的断点列表
void scan_breakpoint();             //扫描所有使用中的断点，将目标地址的首字节替换并保存
bool check_breakpoint();            //扫描所有使用中的断点，若触发则返回true，反之返回false
void replace(vaddr_t addr);        //将给定地址的指令首字节替换回原字节
#endif
