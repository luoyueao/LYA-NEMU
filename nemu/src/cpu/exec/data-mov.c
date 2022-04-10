#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);              //将id_dest->val压栈

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val);               //出栈并用id_dest->val取弹出元素
  operand_write(id_dest, &id_dest->val);//修改目的操作数源地址的值
  
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtlreg_t temp;
  rtl_lr_l(&temp, R_ESP);       //Temp ← (ESP)
  rtl_lr_l(&t0, R_EAX);
  rtl_push(&t0);                // Push(EAX)
  rtl_lr_l(&t0, R_ECX);
  rtl_push(&t0);                // Push(ECX)
  rtl_lr_l(&t0, R_EDX);
  rtl_push(&t0);                // Push(EDX)
  rtl_lr_l(&t0, R_EBX);
  rtl_push(&t0);                // Push(EBX)
  rtl_push(&temp);              // Push(Temp)
  rtl_lr_l(&t0, R_EBP);
  rtl_push(&t0);                // Push(EBP)
  rtl_lr_l(&t0, R_ESI);
  rtl_push(&t0);                // Push(ESI)
  rtl_lr_l(&t0, R_EDI);
  rtl_push(&t0);                // Push(EDI)

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&t0);                 //EDI ← Pop()
  rtl_sr_l(R_EDI, &t0);
  rtl_pop(&t0);                 //ESI ← Pop()
  rtl_sr_l(R_ESI, &t0);
  rtl_pop(&t0);                 //EBP ← Pop()
  rtl_sr_l(R_EBP, &t0);
  rtl_pop(&t0);                 //throwaway ← Pop()(* Skip ESP *)
  rtl_pop(&t0);                 //EBX ← Pop()
  rtl_sr_l(R_EBX, &t0);
  rtl_pop(&t0);                 //EDX ← Pop()
  rtl_sr_l(R_EDX, &t0);
  rtl_pop(&t0);                 //ECX ← Pop()
  rtl_sr_l(R_ECX, &t0);
  rtl_pop(&t0);                 //EAX ← Pop()
  rtl_sr_l(R_EAX, &t0);

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr_l(&t1, R_EBP);             //用t1读取ebp的值
  rtl_sr_l(R_ESP, &t1);             //t1存入esp中

  rtl_pop(&t2);                     //用t2取出栈顶元素并弹栈
  rtl_sr_l(R_EBP, &t2);             //将栈顶元素存入ebp中

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {  //若操作数是16位
    rtl_lr_w(&t1, R_AX);              //将ax内容读到t1中
    rtl_slt(&t3, &t1, &tzero);        //t3 = t1 < 0 ? 1 : 0
    if(t3)
      rtl_li(&t2, 0xFFFF);            //t2 = 0xFFFF
    else
      rtl_li(&t2, 0);                 //t2 = 0
    rtl_sr_w(R_DX, &t2);              //将t2存入dx中
  }
  else {                              //若操作数是32位
    rtl_lr_l(&t1, R_EAX);             //将eax内容读到t1中
    rtl_slt(&t3, &t1, &tzero);        //t3 = t1 < 0 ? 1 : 0
    if(t3)
      rtl_li(&t2, 0xFFFFFFFF);        //t2 = 0xFFFFFFFF
    else
      rtl_li(&t2, 0);                 //t2 = 0
    rtl_sr_l(R_EDX, &t2);             //将t2存入edx中
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr_b(&t1, R_AL);              //将al内容读到t1中
    rtl_sext(&t2, &t1, 1);            //将t1从8位扩展成16位，存入t2
    rtl_sr_w(R_AX, &t2);              //将t2存入ax中
  }
  else {
    rtl_lr_w(&t1, R_AX);              //将ax内容读到t1中
    rtl_sext(&t2, &t1, 2);            //将t1从16位扩展成32位，存入t2
    rtl_sr_l(R_EAX, &t2);             //将t2存入eax中
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
