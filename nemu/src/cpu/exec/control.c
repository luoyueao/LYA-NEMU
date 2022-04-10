#include "cpu/exec.h"

make_EHelper(jmp) {
  // 
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {

  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {

  decoding.is_jmp = 1;                            //跳转标志置1
  rtl_add(&decoding.jmp_eip, eip, &id_dest->val); //设置跳转目标地址
  rtl_li(&t2, *eip);
  rtl_push(&t2);                                  //将下一条指令的地址压栈

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  decoding.is_jmp = 1;  //跳转标志置1
  rtl_pop(&decoding.jmp_eip); //出栈并将出栈地址设置为跳转地址
  if(decoding.opcode == 0xc2 || decoding.opcode == 0xca) {
    cpu.esp += id_dest->val;
  }

  print_asm("ret");
}

make_EHelper(call_rm) {
  decoding.is_jmp = 1;                        //跳转标志置1
  decoding.jmp_eip = id_dest->val;            //设置跳转目标地址
  rtl_li(&t2, *eip);
  rtl_push(&t2);                              //将下一条指令的地址压栈

  print_asm("call *%s", id_dest->str);
}
