// 用 UART 與宿主機溝通的模組
// low-level driver routines for 16550a UART.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// the UART control registers are memory-mapped
// at address UART0. this macro returns the
// address of one of the registers.
#define Reg(reg) ((volatile unsigned char *)(UART0 + reg)) // 讀取 UART 中的暫存器

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                 // receive holding register (for input bytes) (接收暫存器)
#define THR 0                 // transmit holding register (for output bytes) (傳送暫存器)
#define IER 1                 // interrupt enable register (中斷控制暫存器)
#define IER_RX_ENABLE (1<<0)  // (RX 可接收)
#define IER_TX_ENABLE (1<<1)  // (TX 可傳送)
#define FCR 2                 // FIFO control register (FIFO 控制暫存器)
#define FCR_FIFO_ENABLE (1<<0) // (啟用 FIFO)
#define FCR_FIFO_CLEAR (3<<1) // clear the content of the two FIFOs (清除 FIFO)
#define ISR 2                 // interrupt status register (中斷狀態暫存器)
#define LCR 3                 // line control register (行控制暫存器)
#define LCR_EIGHT_BITS (3<<0) // (離開設定鮑率模式, 8 bits, no parity)
#define LCR_BAUD_LATCH (1<<7) // special mode to set baud rate (進入設定鮑率模式)
#define LSR 5                 // line status register (行狀態暫存器)
#define LSR_RX_READY (1<<0)   // input is waiting to be read from RHR (是否有資料可讀)
#define LSR_TX_IDLE (1<<5)    // THR can accept another character to send (目前是否可送)

#define ReadReg(reg) (*(Reg(reg))) // 讀取 UART 暫存器的函數
#define WriteReg(reg, v) (*(Reg(reg)) = (v)) // 寫入 UART 暫存器的函數

// the transmit output buffer.
struct spinlock uart_tx_lock; // tx 的鎖
#define UART_TX_BUF_SIZE 32 // 緩衝區大小
char uart_tx_buf[UART_TX_BUF_SIZE]; // UART 傳送緩衝區
uint64 uart_tx_w; // write next to uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE] (寫的位置)
uint64 uart_tx_r; // read next from uart_tx_buf[uar_tx_r % UART_TX_BUF_SIZE] (讀的位置)

extern volatile int panicked; // from printf.c

void uartstart();

void
uartinit(void) // 初始化 UART ，設定鮑率，啟動 FIFO
{
  // disable interrupts.
  WriteReg(IER, 0x00);

  // special mode to set baud rate.
  WriteReg(LCR, LCR_BAUD_LATCH);

  // LSB for baud rate of 38.4K.
  WriteReg(0, 0x03);

  // MSB for baud rate of 38.4K.
  WriteReg(1, 0x00);

  // leave set-baud mode,
  // and set word length to 8 bits, no parity.
  WriteReg(LCR, LCR_EIGHT_BITS);

  // reset and enable FIFOs.
  WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

  // enable transmit and receive interrupts.
  WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);

  initlock(&uart_tx_lock, "uart");
}

// add a character to the output buffer and tell the
// UART to start sending if it isn't already.
// blocks if the output buffer is full.
// because it may block, it can't be called
// from interrupts; it's only suitable for use
// by write().
void
uartputc(int c) // 送出一個字 (user 用 -- 中斷版)
{
  acquire(&uart_tx_lock);

  if(panicked){
    for(;;)
      ;
  }

  while(1){
    if(uart_tx_w == uart_tx_r + UART_TX_BUF_SIZE){
      // buffer is full.
      // wait for uartstart() to open up space in the buffer.
      sleep(&uart_tx_r, &uart_tx_lock);
    } else {
      uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE] = c;
      uart_tx_w += 1;
      uartstart();
      release(&uart_tx_lock);
      return;
    }
  }
}

// alternate version of uartputc() that doesn't 
// use interrupts, for use by kernel printf() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void
uartputc_sync(int c) // 送出一個字 (kernel 用 -- 不中斷版)
{
  push_off();

  if(panicked){
    for(;;)
      ;
  }

  // wait for Transmit Holding Empty to be set in LSR.
  while((ReadReg(LSR) & LSR_TX_IDLE) == 0)
    ;
  WriteReg(THR, c);

  pop_off();
}

// if the UART is idle, and a character is waiting
// in the transmit buffer, send it.
// caller must hold uart_tx_lock.
// called from both the top- and bottom-half.
void
uartstart() // 若緩衝區有字就送出
{
  while(1){
    if(uart_tx_w == uart_tx_r){
      // transmit buffer is empty.
      return;
    }
    
    if((ReadReg(LSR) & LSR_TX_IDLE) == 0){
      // the UART transmit holding register is full,
      // so we cannot give it another byte.
      // it will interrupt when it's ready for a new byte.
      return;
    }
    
    int c = uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE];
    uart_tx_r += 1;
    
    // maybe uartputc() is waiting for space in the buffer.
    wakeup(&uart_tx_r);
    
    WriteReg(THR, c);
  }
}

// read one input character from the UART.
// return -1 if none is waiting.
int
uartgetc(void) // 讀入一個字
{
  if(ReadReg(LSR) & 0x01){
    // input data is ready.
    return ReadReg(RHR);
  } else {
    return -1;
  }
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from trap.c.
void
uartintr(void) // UART 中斷 (因為有字送進來了)
{
  // read and process incoming characters.
  while(1){
    int c = uartgetc(); // 讀取一個字元
    if(c == -1)
      break;
    consoleintr(c); // 呼叫 consoleintr 處理該字元。
  }

  // send buffered characters.
  acquire(&uart_tx_lock);
  uartstart();
  release(&uart_tx_lock);
}
