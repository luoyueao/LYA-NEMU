#include "cpu/rtl.h"

//状态码

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // dest <- ( cc is satisfied ? 1 : 0)
  rtlreg_t t4, t5, t6;

  switch (subcode & 0xe) {
    case CC_O:            //JO : OF = 1
      rtl_get_OF(dest);
      break;
    case CC_B:            //JB : CF = 1
      rtl_get_CF(dest);
      break;
    case CC_E:            //JE : ZF = 1
      rtl_get_ZF(dest);
      break;
    case CC_BE:           //JBE : (CF or ZF) = 1
      rtl_get_CF(&t4);
      rtl_get_ZF(&t5);
      rtl_or(dest, &t4, &t5);
      break;
    case CC_S:            //JS : SF = 1
      rtl_get_SF(dest);
      break;
    case CC_L:            //JL : (SF xor OF) = 1
      rtl_get_SF(&t4);
      rtl_get_OF(&t5);
      rtl_xor(dest, &t4, &t5);
      break;
    case CC_LE:           //JLE : ((SF xor OF) or ZF) = 1
      rtl_get_SF(&t4);
      rtl_get_OF(&t5);
      rtl_get_ZF(&t6);
      rtl_xor(&t3, &t4, &t5);
      rtl_or(dest, &t3, &t6);
      break;
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }

  if (invert) {   //若是JN*，则只需将结果置反即可
    rtl_xori(dest, dest, 0x1);
  }
}
