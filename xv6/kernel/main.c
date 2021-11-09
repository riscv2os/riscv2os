#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main() // kernel 的主程式，啟動核心並載入第一個 init 使用者行程
{
  if(cpuid() == 0){  // 第 0 的 hart 用來跑核心
    consoleinit();   // 準備好 console
    printfinit();    // 準備好 printf
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator // 準備好實體分頁
    kvminit();       // create kernel page table // 準備好核心分頁表
    kvminithart();   // turn on paging // 啟動分頁表
    procinit();      // process table // 準備好行程表
    trapinit();      // trap vectors  // 設定好 trap 中斷
    trapinithart();  // install kernel trap vector  // 安裝核心的中斷向量
    plicinit();      // set up interrupt controller // 設定中斷控制器
    plicinithart();  // ask PLIC for device interrupts // 設定裝置中斷
    binit();         // buffer cache // 檔案系統: 緩衝快取
    iinit();         // inode cache  // 檔案系統: inode快取
    fileinit();      // file table   // 檔案系統: 設置檔案表
    virtio_disk_init(); // emulated hard disk // 檔案系統: 設置 virtio 虛擬硬碟
    userinit();      // first user process // 啟動第一個使用者行程 init
    __sync_synchronize();
    started = 1;     // 啟動已完成
  } else {  // 其他的 hart 用來跑一般程式
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging // 啟動分頁表
    trapinithart();   // install kernel trap vector  // 安裝核心的中斷向量
    plicinithart();   // ask PLIC for device interrupts // 設定裝置中斷
  }

  scheduler(); // 進入排程系統 (無窮迴圈)
}
