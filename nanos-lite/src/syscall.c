#include "common.h"
#include "syscall.h"
#include "fs.h"

int mm_brk(uint32_t new_brk);

static inline uintptr_t sys_open(uintptr_t pathname, uintptr_t flags, uintptr_t mode) {
//  Log("sys_open : pathname:%d , flags:%d , mode:%d", pathname, flags, mode);
  return fs_open((char *)pathname, flags, mode);  //调用fs_open()并返回其返回值，flags和mode可忽略
}

static inline uintptr_t sys_write(uintptr_t fd, uintptr_t buf, uintptr_t len) {
//  Log("sys_write : fd:%d , buf:%d , len:%d", fd, buf, len);
  return fs_write(fd, (void *)buf, len);  //调用fs_write()并返回其返回值
}

static inline uintptr_t sys_read(uintptr_t fd, uintptr_t buf, uintptr_t len) {
//  Log("sys_read : fd:%d , buf:%d , len:%d", fd, buf, len);
  return fs_read(fd, (void *)buf, len);   //调用fs_read()并返回其返回值
}

static inline uintptr_t sys_lseek(uintptr_t fd, uintptr_t offset, uintptr_t whence) {
//  Log("sys_lseek : fd:%d , offset:%d , whence:%d", fd, offset, whence);
  return fs_lseek(fd, offset, whence);
}

static inline uintptr_t sys_close(uintptr_t fd) {
// Log("sys_close : fd:%d", fd);
  return fs_close(fd);                    //调用fs_close()并返回其返回值，其实就是返回0
}

static inline uintptr_t sys_brk(uintptr_t new_brk) {
//  Log("sys_brk : new_brk:%d", new_brk);
  // return 0;     //总是返回0即可
  return mm_brk(new_brk);                  //调用mm_brk()并返回其值
}

static inline uintptr_t sys_none() {
//  Log("sys_none !");
  return 1;     //不做任何操作，直接返回1
}

static inline uintptr_t sys_exit(uintptr_t code) {
//  Log("sys_exit : code:%d", code);
  _halt(code);  //用参数code调用_halt()
  return 1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);         //a[0] : system call ID
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none : SYSCALL_ARG1(r) = sys_none(); break;  //触发系统调用SYS_none，调用sys_none()，设置系统调用的返回值
    case SYS_exit : sys_exit(a[1]); break;                //触发系统调用SYS_exit，调用sys_exit()
    case SYS_write : SYSCALL_ARG1(r) = sys_write(a[1], a[2], a[3]); break;  //触发系统调用SYS_write，调用sys_write()，设置系统调用的返回值
    case SYS_brk : SYSCALL_ARG1(r) = sys_brk(a[1]); break;//触发系统调用SYS_brk，调用sys_brk()，设置系统调用返回值
    case SYS_open : SYSCALL_ARG1(r) = sys_open(a[1], a[2], a[3]); break; //触发系统调用SYS_open，调用sys_open()，设置系统调用的返回值
    case SYS_read : SYSCALL_ARG1(r) = sys_read(a[1], a[2], a[3]); break; //触发系统调用SYS_read，调用sys_read()，设置系统调用的返回值
    case SYS_lseek : SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]); break; //触发系统调用SYS_lseek，调用sys_lseek()，设置系统调用的返回值
    case SYS_close : SYSCALL_ARG1(r) = sys_close(a[1]); break;  //触发系统调用SYS_close，调用sys_close()，设置系统调用的返回值
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
