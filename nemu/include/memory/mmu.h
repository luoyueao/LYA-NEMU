#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>

//页大小4KB
#define PAGE_SIZE					4096
#define NR_PDE						1024
#define NR_PTE						1024
#define PAGE_MASK					(4096 - 1)
#define PT_SIZE						((NR_PTE) * (PAGE_SIZE))

//CR0寄存器
typedef union CR0 {
  struct {
    uint32_t protect_enable      : 1;
    uint32_t dont_care           : 30;
    uint32_t paging              : 1;
  };
  uint32_t val;
} CR0;

//CR3/
typedef union CR3 {
  struct {
    uint32_t pad0                : 3;
    uint32_t page_write_through  : 1;
    uint32_t page_cache_disable  : 1;
    uint32_t pad1                : 7;
    uint32_t page_directory_base : 20;
  };
  uint32_t val;
} CR3;


//页目录结构
typedef union PageDirectoryEntry {
  struct {
    uint32_t present             : 1;
    uint32_t read_write          : 1; 
    uint32_t user_supervisor     : 1;
    uint32_t page_write_through  : 1;
    uint32_t page_cache_disable  : 1;
    uint32_t accessed            : 1;
    uint32_t pad0                : 6;
    uint32_t page_frame          : 20;
  };
  uint32_t val;
} PDE;

//页表结构
typedef union PageTableEntry {
  struct {
    uint32_t present             : 1;
    uint32_t read_write          : 1;
    uint32_t user_supervisor     : 1;
    uint32_t page_write_through  : 1;
    uint32_t page_cache_disable  : 1;
    uint32_t accessed            : 1;
    uint32_t dirty               : 1;
    uint32_t pad0                : 1;
    uint32_t global              : 1;
    uint32_t pad1                : 3;
    uint32_t page_frame          : 20;
  };
  uint32_t val;
} PTE;

typedef PTE (*PT) [NR_PTE];

typedef union GateDescriptor {
  struct {
    uint32_t offset_15_0      : 16;
    uint32_t dont_care0       : 16;
    uint32_t dont_care1       : 15;
    uint32_t present          : 1;
    uint32_t offset_31_16     : 16;
  };
  uint32_t val;
} GateDesc;

#endif
