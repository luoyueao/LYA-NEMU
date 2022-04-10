#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  

  rtl_push(&cpu.eflags.val);          //将eflags压栈
  cpu.eflags.IF = 0;                  //处理器进入关中断状态
  rtl_push(&cpu.cs);                  //将cs压栈
  rtl_push(&ret_addr);                //将int指令的下一条指令地址压栈

  vaddr_t idt_addr, low, high;
  idt_addr = cpu.idtr.base + 8 * NO;  //因为门描述符的大小为8个字节，所以通过base+8*NO来索引
  low = vaddr_read(idt_addr, 2);      //low取门描述符低16位
  high = vaddr_read(idt_addr + 4, 4);
  high = high & 0xffff0000;           //high取门描述符高16位
  decoding.is_jmp = 1;
  decoding.jmp_eip = high | low;      //组合low与high为跳转目标地址
}

void dev_raise_intr() {
  cpu.INTR = 1;
}
