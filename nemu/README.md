# NEMU


Small x86 programs can run under NEMU.

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with QEMU
* CPU core with support of most common used x86 instructions in protected mode
* DRAM
* I386 paging with TLB
  * protection is not supported
* I386 interrupt and exception
  * protection is not supported
*  devices
  * timer, keyboard, VGA
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O
