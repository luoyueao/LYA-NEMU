#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];


uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id;
  mmio_id = is_mmio(addr);                //获取映射信号
  if(mmio_id != -1)
    return mmio_read(addr, len, mmio_id); //若映射信号有效，则调用mmio_read
  else
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3)); //若映射信号无效，则调用pmem_rw
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_id;
  mmio_id = is_mmio(addr);                //获取映射信号
  if(mmio_id != -1)
    return mmio_write(addr, len, data, mmio_id);  //若映射信号有效，则调用mmio_write
  else
    memcpy(guest_to_host(addr), &data, len);      //若映射信号无效，则调用pmem_rw
}

paddr_t page_translate(vaddr_t vaddr, bool write_flag) {
  PDE pde;
  PTE pte;
  // Log("vaddr : %#x", vaddr);

  //页目录的基地址左移12位作高位，vaddr取高10位并乘4作低位
  uint32_t pde_addr = (cpu.cr3.page_directory_base << 12) + ((vaddr >> 22) << 2);
  pde.val = paddr_read(pde_addr, 4);  //读出PDE
  assert(pde.present);                //检查 present 位
  // Log("cpu.cr3.page_directory_base : %#x", cpu.cr3.page_directory_base);
  // Log("DIR : %#x", (vaddr >> 22));
  // Log("pde_addr : %#x", pde_addr);

  //页目录项中的page_frame左移12位作高位(即高20位)，vaddr取中10位并乘4作低位
  uint32_t pte_addr = (pde.page_frame << 12) + (((vaddr >> 12) & 0x3ff) << 2);
  pte.val = paddr_read(pte_addr, 4);  //读出PTE
  assert(pte.present);                //检查 present 位
  // Log("pde.page_frame : %#x", pde.page_frame);
  // Log("PAGE : %#x", ((vaddr >> 12) & 0x3ff));
  // Log("pte_addr : %#x", pte_addr);

  //页表项中的page_frame左移12位作高位(即高20位)，vaddr取低12位
  uint32_t paddr = (pte.page_frame << 12) + (vaddr & 0xfff);
  // Log("pte.page_frame : %#x", pte.page_frame);
  // Log("OFFSET : %#x", (vaddr & 0xfff));
  // Log("paddr : %#x", paddr);

  //如果pde.accessed为 0 则需变为 1，并写回到页目录项所在地址
  if(!pde.accessed) {
    pde.accessed = 1;
    paddr_write(pde_addr, 4, pde.val);
  }

  //如果pte.accessed为 0 或者 pte.dirty 为 0 且 正在写操作，则更新 accessed 和 dirty，并写回页表项所在地址
  if(!pte.accessed || (!pte.dirty && write_flag)) {
    pte.accessed = 1;
    pte.dirty = 1;
    paddr_write(pte_addr, 4, pte.val);
  }

  return paddr;                   //返回物理地址
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {                  //开启分页机制
    if (((addr & 0xfff) + len) > 4096) {//数据跨越虚拟页边界的情况

      int front_len, back_len;
      paddr_t front_addr, back_addr;
      uint32_t front_data, back_data;

      front_len = 4096 - (addr & 0xfff);//前一个页面上数据的长度
      back_len = len - front_len;       //后一个页面上数据的长度

      front_addr = page_translate(addr, 0);           //前一个页面上数据的起始地址
      back_addr = page_translate(addr + front_len, 0);//后一个页面上数据的起始地址

      front_data = paddr_read(front_addr, front_len); //前一个页面上的数据
      back_data = paddr_read(back_addr, back_len);    //后一个页面上的数据

      return front_data + (back_data << (front_len << 3)); //将后一个页面上的数据作为前一个页面上数据的高位（字节数 * 8 = 位数）
    }
    else {                              //未出现跨页情况，通过page_translate()转换
      paddr_t paddr = page_translate(addr, 0);
      return paddr_read(paddr, len);
    }
  }
  else                                  //未开启分页机制
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {                  //开启分页机制
    if (((addr & 0xfff) + len) > 4096) {//数据跨越虚拟页边界的情况

      int front_len, back_len;
      paddr_t front_addr, back_addr;

      front_len = 4096 - (addr & 0xfff);//前一个页面上数据的长度
      back_len = len - front_len;       //后一个页面上数据的长度

      front_addr = page_translate(addr, 1);           //前一个页面上数据的起始地址
      back_addr = page_translate(addr + front_len, 1);//后一个页面上数据的起始地址

      paddr_write(front_addr, front_len, data);       //前一个页面上的数据写入
      paddr_write(back_addr, back_len, (data >> (front_len << 3)));//后一个页面上的数据写入
    }
    else {                              //未出现跨页情况，通过page_translate()转换
      paddr_t paddr = page_translate(addr, 1);
      paddr_write(paddr, len, data);
    }
  }
  else                                  //未开启分页机制
    paddr_write(addr, len, data);
}
