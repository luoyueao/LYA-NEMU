#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);             //声明do_syscall()
_RegSet* schedule(_RegSet *prev);                   //声明schedule()

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL :                           //识别事件_EVENT_SYSCALL，调用do_syscall()，再调用 schedule() 并返回其现场 
      do_syscall(r); 
      break;
    case _EVENT_TRAP :                              //识别事件_EVENT_TRAP，调用 schedule() 并返回其现场
      return schedule(r);
    case _EVENT_IRQ_TIME :                          //识别事件_EVENT_IRQ_TIME，调用 schedule() 并返回其现场
      return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
