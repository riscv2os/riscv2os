
# RISC-V QEMU Virt

* https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c

```cpp
static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =       {        0x0,         0x100 },
    [VIRT_MROM] =        {     0x1000,        0xf000 },
    [VIRT_TEST] =        {   0x100000,        0x1000 },
    [VIRT_RTC] =         {   0x101000,        0x1000 },
    [VIRT_CLINT] =       {  0x2000000,       0x10000 },
    [VIRT_ACLINT_SSWI] = {  0x2F00000,        0x4000 },
    [VIRT_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [VIRT_PLIC] =        {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_UART0] =       { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =      { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =      { 0x10100000,          0x18 },
    [VIRT_FLASH] =       { 0x20000000,     0x4000000 },
    [VIRT_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =        { 0x80000000,           0x0 },
};
```

* PCIE 总线协议
    * [PCIE学习笔记(一)](https://zhuanlan.zhihu.com/p/216034122)
    * [PCIe 体系结构简介](https://zhuanlan.zhihu.com/p/283026221)

## 

* [RISCV AI SOC实战（一，概述及架构设计）](https://zhuanlan.zhihu.com/p/161010994)
* [RISCV AI SOC实战（二，SOC的时钟管理）](https://zhuanlan.zhihu.com/p/161194737)
* [RISCV AI SOC实战（三，SOC的功耗管理）](https://zhuanlan.zhihu.com/p/161787237)
* [RISCV AI SOC实战（四，复位管理）](https://zhuanlan.zhihu.com/p/161891614)
* [RISCV AI SOC实战（五，中断管理）](https://zhuanlan.zhihu.com/p/162888552)

![](https://pic2.zhimg.com/80/v2-46ca43595e3625c5aa38d7b6be395a61_1440w.jpg)

摘要：

中断流程只要是CPU都大同小异的。对于RISC-V来讲，中断流程是这样的。

1. 外设发出中断信号。
2. PLIC或者CLINT响应中断，RISC-V核心保存此时的CSR（control and status registers，包括了PC啊，中断原因啊一堆信息）。
3. 跳转到中断处理程序（直接换PC值取指令即可）。
4. 关闭其他中断响应使能（RISC-V不支持嵌套，所以一个中断要屏蔽其他中断）。
5. 软件保存通用的寄存器。
6. 然后处理中断（过程中会清掉外设的中断）。
7. 软件恢复通用的寄存器。
8. 然后回复CSR。
9. 然后跳转PC跳回原来位置退出异常。
