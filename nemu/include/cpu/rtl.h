#ifndef __RTL_H__
#define __RTL_H__

#include "nemu.h"

extern rtlreg_t t0, t1, t2, t3;
extern const rtlreg_t tzero;

//基本RTL指令

static inline void rtl_li(rtlreg_t* dest, uint32_t imm) {
  *dest = imm;
}

#define c_add(a, b) ((a) + (b))
#define c_sub(a, b) ((a) - (b))
#define c_and(a, b) ((a) & (b))
#define c_or(a, b)  ((a) | (b))
#define c_xor(a, b) ((a) ^ (b))
#define c_shl(a, b) ((a) << (b))
#define c_shr(a, b) ((a) >> (b))
#define c_sar(a, b) ((int32_t)(a) >> (b))
#define c_slt(a, b) ((int32_t)(a) < (int32_t)(b))
#define c_sltu(a, b) ((a) < (b))

#define make_rtl_arith_logic(name) \
  static inline void concat(rtl_, name) (rtlreg_t* dest, const rtlreg_t* src1, const rtlreg_t* src2) { \
    *dest = concat(c_, name) (*src1, *src2); \
  } \
  static inline void concat3(rtl_, name, i) (rtlreg_t* dest, const rtlreg_t* src1, int imm) { \
    *dest = concat(c_, name) (*src1, imm); \
  }


make_rtl_arith_logic(add)
make_rtl_arith_logic(sub)
make_rtl_arith_logic(and)
make_rtl_arith_logic(or)
make_rtl_arith_logic(xor)
make_rtl_arith_logic(shl)
make_rtl_arith_logic(shr)
make_rtl_arith_logic(sar)
make_rtl_arith_logic(slt)
make_rtl_arith_logic(sltu)

static inline void rtl_mul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("mul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_imul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("imul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_div(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("div %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_idiv(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("idiv %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_lm(rtlreg_t *dest, const rtlreg_t* addr, int len) {
  *dest = vaddr_read(*addr, len);
}

static inline void rtl_sm(rtlreg_t* addr, int len, const rtlreg_t* src1) {
  vaddr_write(*addr, len, *src1);
}

static inline void rtl_lr_b(rtlreg_t* dest, int r) {
  *dest = reg_b(r);
}

static inline void rtl_lr_w(rtlreg_t* dest, int r) {
  *dest = reg_w(r);
}

static inline void rtl_lr_l(rtlreg_t* dest, int r) {
  *dest = reg_l(r);
}

static inline void rtl_sr_b(int r, const rtlreg_t* src1) {
  reg_b(r) = *src1;
}

static inline void rtl_sr_w(int r, const rtlreg_t* src1) {
  reg_w(r) = *src1;
}

static inline void rtl_sr_l(int r, const rtlreg_t* src1) {
  reg_l(r) = *src1;
}



static inline void rtl_lr(rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_lr_l(dest, r); return;
    case 1: rtl_lr_b(dest, r); return;
    case 2: rtl_lr_w(dest, r); return;
    default: assert(0);
  }
}

static inline void rtl_sr(int r, int width, const rtlreg_t* src1) {
  switch (width) {
    case 4: rtl_sr_l(r, src1); return;
    case 1: rtl_sr_b(r, src1); return;
    case 2: rtl_sr_w(r, src1); return;
    default: assert(0);
  }
}

#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    cpu.eflags.f = *src; \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    *dest = cpu.eflags.f; \
  }

make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)

static inline void rtl_mv(rtlreg_t* dest, const rtlreg_t *src1) {
  // 将源操作数赋值给目的操作数
  // dest <- src1
  rtl_or(dest, src1, &tzero);       //*dest = *src1 | 0
}

static inline void rtl_not(rtlreg_t* dest) {
  // 将目的操作数取反
  // dest <- ~dest
  rtl_xori(dest, dest, 0xffffffff); //*dest = *dest ^ 0xffffffff
}

static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // 将源操作数按符号位扩展，赋值给目的操作数
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  rtlreg_t r0;
  rtl_li(&r0, 32 - 8 * width);      //r0 = 将符号位移到第31位所需移动的位数
  rtl_shl(dest, src1, &r0);         //*dest = *src1 << r0，逻辑左移r0
  rtl_sar(dest, dest, &r0);         //*dest = *dest >> r0，算术右移r0
}

static inline void rtl_push(const rtlreg_t* src1) {
  // 将源操作数入栈
  // esp <- esp - 4
  // M[esp] <- src1
  rtlreg_t r0;
  rtl_lr_l(&r0, R_ESP);             //r0读取esp的值
  rtl_subi(&r0, &r0, 4);            //r0 = r0 - 4
  rtl_sr_l(R_ESP, &r0);             //r0存入esp中
  rtl_sm(&r0, 4, src1);             //调用rtl_sm向内存存储
}

static inline void rtl_pop(rtlreg_t* dest) {
  // 出栈并将出栈元素赋给目的操作数
  // dest <- M[esp]
  // esp <- esp + 4
  rtlreg_t r0;
  rtl_lr_l(&r0, R_ESP);             //r0读取esp的值
  rtl_lm(dest, &r0, 4);             //调用rtl_lm对内存读取
  rtl_addi(&r0, &r0, 4);            //r0 = r0 + 4
  rtl_sr_l(R_ESP, &r0);             //r0存入esp中
}

static inline void rtl_eq0(rtlreg_t* dest, const rtlreg_t* src1) {
  // 若源操作数等于0，则目的操作数赋值1，反之0
  // dest <- (src1 == 0 ? 1 : 0)
  rtlreg_t r0;
  rtl_sltu(&r0, &tzero, src1);      //r0 = 0 < *src1，无符号数比较
  rtl_xori(dest, &r0, 1);           //*dest = r0 ^ 1，将r0结果0，1反置
}

static inline void rtl_eqi(rtlreg_t* dest, const rtlreg_t* src1, int imm) {
  // 若源操作数等于imm，则目的操作数赋值1，反之0
  // dest <- (src1 == imm ? 1 : 0)
  rtlreg_t r0;
  rtl_xori(&r0, src1, imm);         //r0 = *src1 ^ imm
  rtl_eq0(dest, &r0);               //若r0等于0，则赋值1，反之赋值0
}

static inline void rtl_neq0(rtlreg_t* dest, const rtlreg_t* src1) {
  // 若源操作数不等于0，则目的操作数赋值1，反之0
  // dest <- (src1 != 0 ? 1 : 0)
  rtl_sltu(dest, &tzero, src1);     //*dest = 0 < *src1，无符号数比较
}

static inline void rtl_msb(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // 将源操作数符号位赋值给目的操作数
  // dest <- src1[width * 8 - 1]
  rtlreg_t r0;
  rtl_shri(&r0, src1, width * 8 - 1); //r0 = *src1 >> (width * 8 - 1)
  rtl_andi(dest, &r0, 1);             //*dest = r0 & 1
}

static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  // 根据*result更新ZF标志
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  rtlreg_t r0;
  rtl_shli(&r0, result, 32 - 8 * width);  //r0 = *result << (32 - 8 * width)
  rtl_eq0(&r0, &r0);                      //若r0等于0，则赋值1，反之赋值0
  rtl_set_ZF(&r0);                        //调用rtl_set_ZF设置ZF标志
}

static inline void rtl_update_SF(const rtlreg_t* result, int width) {
  // 根据*result更新SF标志
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  rtlreg_t r0;
  rtl_msb(&r0, result, width);            //调用上述实现的取符号位的函数，将符号取到r0中
  rtl_set_SF(&r0);                        //调用rtl_set_SF设置SF标志
}

static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
  rtl_update_ZF(result, width);
  rtl_update_SF(result, width);
}

#endif
