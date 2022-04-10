#include "cpu/exec.h"
#include "monitor/watchpoint.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  if(decoding.is_operand_size_16) {                     //若操作数是16位
    cpu.idtr.limit = vaddr_read(id_dest->addr, 2);      //limit取低16位
    cpu.idtr.base = vaddr_read(id_dest->addr + 2, 3);   //base取24位，高8位不取
  }
  else {                                                //若操作数是32位
    cpu.idtr.limit = vaddr_read(id_dest->addr, 2);      //limit取低16位
    cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);   //base取32位，高8位取
  }
  
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  switch(id_dest->reg) {
    case 0:       //目的寄存器是CR0
      cpu.cr0.val = id_src->val;
      break;
    case 3:       //目的寄存器是CR3
      cpu.cr3.val = id_src->val;
      break;
    default:
      assert(0);
  }

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  operand_write(id_dest, &id_src->val); // DEST ← SRC

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);   //调用raise_intr()函数来实现中断

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(int3) {
  decoding.seq_eip --;                  //顺序取指需要回退一个字节，为下一次重新读取做准备
  replace(decoding.seq_eip);            //将断点处替换回原字节

  print_asm("int 3");
}

make_EHelper(iret) {
  decoding.is_jmp = 1;
  rtl_pop(&decoding.jmp_eip);           //Pop(eip)
  rtl_pop(&cpu.cs);                     //Pop(cs)
  rtl_pop(&cpu.eflags.val);             //Pop(eflags)

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t2 = pio_read(id_src->val, id_dest->width);   //根据源操作数所给信息将数据读到目的操作数中
  operand_write(id_dest, &t2);                  //将所读数据写入目的操作数源地址

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_dest->width, id_src->val);   //将源操作数数据写入目的端口

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
