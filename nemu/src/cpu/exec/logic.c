#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);            //将dest和src相与，结果存入t2，后丢弃

  rtl_set_OF(&tzero);                                   //OF = 0
  rtl_set_CF(&tzero);                                   //CF = 0

  rtl_update_ZFSF(&t2, id_dest->width);                 //更新ZF、SF标志

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);  //将dest和src相与，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  rtl_set_OF(&tzero);                                   //OF = 0
  rtl_set_CF(&tzero);                                   //CF = 0

  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&id_dest->val, &id_dest->val, &id_src->val);  //将dest和src异或，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址
  
  rtl_set_OF(&tzero);                                   //OF = 0
  rtl_set_CF(&tzero);                                   //CF = 0
  
  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&id_dest->val, &id_dest->val, &id_src->val);   //将dest和src相或，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  rtl_set_OF(&tzero);                                   //OF = 0
  rtl_set_CF(&tzero);                                   //CF = 0
  
  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sar(&id_dest->val, &id_dest->val, &id_src->val);  //将dest算术右移src位，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志


  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&id_dest->val, &id_dest->val, &id_src->val);  //将dest逻辑左移src位，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志
 

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&id_dest->val, &id_dest->val, &id_src->val);  //将dest逻辑右移src位，结果存入dest
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  rtl_update_ZFSF(&id_dest->val, id_dest->width);       //更新ZF、SF标志


  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);                               //将dest按位取反
  operand_write(id_dest, &id_dest->val);                //将结果写入目的地址

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtl_mv(&t1, &id_src->val);                            //左移位数存入t1
  while(t1)                                             //当位移不为0
  {
    rtl_msb(&t2, &id_dest->val, id_dest->width);        //将最高位取到t2
    rtl_shli(&id_dest->val, &id_dest->val, 1);          //dest逻辑左移1位
    rtl_add(&id_dest->val, &id_dest->val, &t2);         //将最高位t2加到左移后的dest的最低位
    rtl_subi(&t1, &t1, 1);                              //位移数减1
  }

  rtl_set_CF(&t2);                                      //将最后一次移出的最高位给CF

  rtl_eqi(&t1, &id_src->val, 1);                        //判断左移总位数与1是否相等
  if(t1)                                                //若等于1
  {
    rtl_msb(&t3, &id_dest->val, id_dest->width);        //将最高位取到t3
    rtl_xor(&t0, &t3, &t2);                             //若最高位不等于CF，则OF置1，反之置0
    rtl_set_OF(&t0);
  }

  operand_write(id_dest, &id_dest->val);                //写入结果
  
  print_asm_template2(rol);
}
