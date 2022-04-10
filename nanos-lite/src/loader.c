#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);  //声明ramdisk_read()
extern size_t get_ramdisk_size();                               //声明get_ramdisk_size()
extern ssize_t fs_read(int fd, void *buf, size_t len);          //声明fs_read()
extern int fs_open(const char *pathname, int flags, int mode);  //声明fs_open()
extern int fs_close(int fd);                                    //声明fs_close()
extern size_t fs_filesz(int fd);                                //声明fs_filesz()

extern void* new_page(void);                                    //声明new_page()

uintptr_t loader(_Protect *as, const char *filename) {
  int fd =  fs_open(filename, 0, 0);      //打开文件filename，获取文件描述符fd，flags与mode参数可忽略
  int len = fs_filesz(fd);                //获取文件大小len
  
  void *pa;
  void *va = DEFAULT_ENTRY;               //起始虚拟地址为DEFAULT_ENTRY

  while(len > 0) {                        //若待装入大小仍大于0
    pa = new_page();                      //申请一个空闲物理页
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);                     //建立映射
    fs_read(fd, pa, 4096);                //读一页内容，写到这个物理页上
    len -= 4096;                          //装入大小减去页大小
    va += 4096;                           //虚拟地址加上页大小
  }

  fs_close(fd);                           //关闭文件
  return (uintptr_t)DEFAULT_ENTRY;        //返回起始虚拟地址
}
