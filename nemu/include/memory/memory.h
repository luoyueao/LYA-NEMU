#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

extern uint8_t pmem[];

//物理地址到虚拟地址转换
#define guest_to_host(p) ((void *)(pmem + (unsigned)p))

#define host_to_guest(p) ((paddr_t)((void *)p - (void *)pmem))

uint32_t vaddr_read(vaddr_t, int);
uint32_t paddr_read(paddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);
void paddr_write(paddr_t, int, uint32_t);

#endif
