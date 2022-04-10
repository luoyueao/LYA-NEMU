#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "memory/memory.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}


WP* new_wp() {
  if(!free_) {        //若没有空闲监视点，则assert(0)
    assert(0);
  }
  else {              //若有空闲监视点，则返回当前free_指向的监视点，并将free_后移一个空闲监视点
    WP *temp;
    temp = free_;
    free_ = free_->next;
    return temp;
  }
}

void free_wp(WP *wp) {
  WP *p, *q;
  if(head == wp) {    //若头节点是删除点，则p指向该点，head后移
    p = head;
    head = head->next;
  }
  else {              //若头节点不是删除点，则q、p一前一后同步移动，直至p指向删除点，q衔接p之后的链表
    for(q = head, p = head->next; p != wp; p = p->next, q = q->next);
    q->next = p->next;
  }
  for(q = free_; q->next; q = q->next);
  p->next = NULL;
  q->next = p;        //将回收的空闲监视点插入free_链表尾
}

int set_watchpoint(char *e) {
  WP *p, *q;
  p = new_wp();                         //申请一个空闲监视点
  strcpy(p->expr, e);                   //存入表达式
  p->old_val = expr(e, NULL);           //存入旧值
  p->next = NULL;
  if(!head)                             //若head为NULL
    head = p;                           //则p称为head
  else {                                //若head不为NULL
    for(q = head; q->next; q = q->next);//找到head链表的尾，将p插入作为新的尾
    q->next = p;
  }
  p->type = 0;                          //设置类型为监视点
  return p->NO;                         //返回编号
}

bool delete_watchpoint(int NO) {
  WP *p;
  for(p = head; p ; p = p->next)        //寻找指定编号的监视点
    if(p->NO == NO)
      break;
  if(!p)                                //若没有找到，则返回false
    return false;
  else {                                //若找到，通过free_wp()函数回收该节点，返回true
    if(p->type == 1)                    //若删除的是断点，则需要重新将原字节归还
      vaddr_write(p->addr, 1, p->sub);
    free_wp(p);
    return true;
  }
}

void list_watchpoint() {
  WP *p;
  printf("NO Expr         Old Value\n");
  for(p = head; p; p = p->next)         //用p指针遍历监视点，格式输出编号、表达式、旧值
    if(p->type == 0)  
      printf("%2d %-12s %#x\n", p->NO, p->expr, p->old_val);
}

WP* scan_watchpoint() {
  WP *p, *q = NULL;                     //p遍历监视点，q指向触发监视点或NULL
  for(p = head; p; p = p->next) {
    if(p->type)
      continue;
    p->new_val = expr(p->expr, NULL);   //计算表达式新值
    if(p->new_val != p->old_val) {      //若监视点新旧值不同
      if(q)                             //若q已经指向了一个触发点，则将指向的监视点的旧值更新
        q->old_val = q->new_val;
      q = p;                            //将q指向新发现的触发点
    }
  }
  return q;                             //返回q，可能是监视点也可能是NULL
}

int set_breakpoint(vaddr_t addr) {
  WP *p, *q;
  p = new_wp();                         //申请一个空闲点
  p->addr = addr;                       //存入目标地址
  p->sub = vaddr_read(addr, 1);         //存入目标地址一个字节的内容
  p->activation = 0;                    //设置未被激活
  p->next = NULL;
  if(!head)                             //若head为NULL
    head = p;                           //则p称为head
  else {                                //若head不为NULL
    for(q = head; q->next; q = q->next);//找到head链表的尾，将p插入作为新的尾
    q->next = p;
  }
  p->type = 1;                          //设置类型为断点
  return p->NO;
}

void list_breakpoint() {
  WP *p;
  printf("NO Addr\n");
  for(p = head; p; p = p->next)         //用p指针遍历断点，格式输出编号、地址
    if(p->type == 1)
      printf("%2d %#x\n", p->NO, p->addr);
}

void scan_breakpoint() {
  WP *p;
  for(p = head; p; p = p->next)         //用p指针遍历断点，将目标地址的首字节替换成0xcc
    if(p->type == 1) {
     if(p->activation) {
        p->activation = 0;              //断点激活过重新置0
        continue;
      }
      vaddr_write(p->addr, 1, 0xcc);
    }
}

bool check_breakpoint() {
  WP *p;
  for(p = head; p; p = p->next)
    if(p->activation) {
      printf("Hit breakpoint %d at %#x\n", p->NO, p->addr);
      return true;                      //若发现断点处的指令首字节被替换回原字节，则返回true
    }
  return false;                         //否则返回false
}

void replace(vaddr_t addr) {
  WP *p;
  for(p = head; p; p = p->next) {       //遍历断点，将给定地址的指令首字节替换回原字节
    if(p->type == 1 && addr == p->addr) {
      vaddr_write(p->addr, 1, p->sub);
      p->activation = 1;                //断点被激活
      break;
    }
  }
}
