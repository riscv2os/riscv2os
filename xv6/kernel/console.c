// 控制台模組 (透過 UART 進行鍵盤輸入與螢幕輸出)
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

//
// send one character to the uart.
// called by printf, and to echo input characters,
// but not from write().
//
void
consputc(int c) // 這個函數是處理畫面的，但若是 write (檔案輸出) 則不應呼叫這個函數。
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync('\b'); uartputc_sync(' '); uartputc_sync('\b'); // 在畫面上 BACKSPACE 的行為會蓋掉前一個字
  } else {
    uartputc_sync(c);
  }
}

struct {
  struct spinlock lock;
  
  // input
#define INPUT_BUF 128
  char buf[INPUT_BUF];
  uint r;  // Read index   (讀取位置)
  uint w;  // Write index  (寫入位置)
  uint e;  // Edit index   (編輯位置)
} cons;

//
// user write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n) //  write() 呼叫的 console 版本
{
  int i;

  for(i = 0; i < n; i++){
    char c;
    if(either_copyin(&c, user_src, src+i, 1) == -1) // 從作業系統取得 src[i] 的字元
      break;
    uartputc(c); // 透過 uart 印出
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n) //  read() 呼叫的 console 版本
{
  uint target;
  int c;
  char cbuf;

  target = n;
  acquire(&cons.lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){ // cons.r == cons.w 代表沒有資料可讀了，只能等待！
      if(myproc()->killed){
        release(&cons.lock);
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r++ % INPUT_BUF]; // 讀取緩衝區的下一個字元

    if(c == C('D')){  // end-of-file (已到檔尾)
      if(n < target){ // 若不是最後一個字，退回一格
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer. (複製該字元到使用者空間)
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
      break;

    dst++;
    --n;

    if(c == '\n'){ // 讀到換行字元，返回 read() 去
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  release(&cons.lock);

  return target - n; // 傳回總共讀了幾個字
}

//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c) // 有字元輸入中斷時，會透過 uartintr() 間接呼叫這個函數。
{
  acquire(&cons.lock);

  switch(c){
  case C('P'):  // Print process list. (若是 Ctrl-P 就印出行程列表)
    procdump();
    break;
  case C('U'):  // Kill line. (若是 Ctrl-U 就刪除最後那行)
    while(cons.e != cons.w &&
          cons.buf[(cons.e-1) % INPUT_BUF] != '\n'){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
  case C('H'): // Backspace 若是 Ctrl-H 則退回一格
  case '\x7f':
    if(cons.e != cons.w){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
  default: // 否則就印出該字元
    if(c != 0 && cons.e-cons.r < INPUT_BUF){
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      consputc(c);

      // store for consumption by consoleread().
      cons.buf[cons.e++ % INPUT_BUF] = c;

      if(c == '\n' || c == C('D') || cons.e == cons.r+INPUT_BUF){ // 若是 \n | Ctrl-D | 緩衝區滿了
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        cons.w = cons.e;
        wakeup(&cons.r); // 就喚醒等待 console 輸入的行程
      }
    }
    break;
  }
  
  release(&cons.lock);
}

void
consoleinit(void) // 初始化本模組
{
  initlock(&cons.lock, "cons");

  uartinit(); // 初始化 UART

  // connect read and write system calls
  // to consoleread and consolewrite.
  // 第一號裝置 devsw[1] 為 console
  devsw[CONSOLE].read = consoleread;    // 設定 console 的 read 函數
  devsw[CONSOLE].write = consolewrite;  // 設定 console 的 write 函數
}
