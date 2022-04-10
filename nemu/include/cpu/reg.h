#ifndef __REG_H__
#define __REG_H__

#include "common.h"
#include "memory/mmu.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };


typedef union {
  union {
    uint32_t _32;
    uint16_t _16;
    uint8_t _8[2];
  } gpr[8];//表示一个32位寄存器


  struct {
    rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;

    vaddr_t eip;

    union {
      struct {
        unsigned int CF : 1;  //CF：1位
        unsigned int : 5;     //空域：5位
        unsigned int ZF : 1;  //ZF：1位
        unsigned int SF : 1;  //SF：1位
        unsigned int : 1;     //空域：1位
        unsigned int IF: 1;   //IF：1位
        unsigned int : 1;     //空域：1位
        unsigned int OF : 1;  //OF：1位
        unsigned int : 20;    //空域：20位
      };
      rtlreg_t val;           //上述32位的结合，用于赋初值
    } eflags;

    struct {
      uint32_t base;          //BASE ：32 位
      uint16_t limit;         //LIMIT ：64 位
    } idtr;                   //IDTR 寄存器

    rtlreg_t cs;              //CS 寄存器

    CR0 cr0;                  //CR0 寄存器
    CR3 cr3;                  //CR3 寄存器

    bool INTR;                //INTR 引脚
  };

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif
