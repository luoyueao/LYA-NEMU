#include "cpu/exec.h"
#include "cpu/rtl.h"

void load_addr(vaddr_t *eip, ModR_M *m, Operand *rm) {
  assert(m->mod != 3);

  int32_t disp = 0;
  int disp_size = 4;
  int base_reg = -1, index_reg = -1, scale = 0;
  rtl_li(&rm->addr, 0);

  if (m->R_M == R_ESP) {
    SIB s;
    s.val = instr_fetch(eip, 1);
    base_reg = s.base;
    scale = s.ss;

    if (s.index != R_ESP) { index_reg = s.index; }
  }
  else {
    /* no SIB */
    base_reg = m->R_M;
  }

  if (m->mod == 0) {
    if (base_reg == R_EBP) { base_reg = -1; }
    else { disp_size = 0; }
  }
  else if (m->mod == 1) { disp_size = 1; }

  if (disp_size != 0) {
    /* has disp */
    disp = instr_fetch(eip, disp_size);
    if (disp_size == 1) { disp = (int8_t)disp; }

    rtl_addi(&rm->addr, &rm->addr, disp);
  }

  if (base_reg != -1) {
    rtl_add(&rm->addr, &rm->addr, &reg_l(base_reg));
  }

  if (index_reg != -1) {
    rtl_shli(&t0, &reg_l(index_reg), scale);
    rtl_add(&rm->addr, &rm->addr, &t0);
  }



  rm->type = OP_TYPE_MEM;
}

void read_ModR_M(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  ModR_M m;
  m.val = instr_fetch(eip, 1);
  decoding.ext_opcode = m.opcode;
  if (reg != NULL) {
    reg->type = OP_TYPE_REG;
    reg->reg = m.reg;
    if (load_reg_val) {
      rtl_lr(&reg->val, reg->reg, reg->width);
    }

#ifdef DEBUG
    snprintf(reg->str, OP_STR_SIZE, "%%%s", reg_name(reg->reg, reg->width));
#endif
  }

  if (m.mod == 3) {
    rm->type = OP_TYPE_REG;
    rm->reg = m.R_M;
    if (load_rm_val) {
      rtl_lr(&rm->val, m.R_M, rm->width);
    }

#ifdef DEBUG
    sprintf(rm->str, "%%%s", reg_name(m.R_M, rm->width));
#endif
  }
  else {
    load_addr(eip, &m, rm);
    if (load_rm_val) {
      rtl_lm(&rm->val, &rm->addr, rm->width);
    }
  }
}
