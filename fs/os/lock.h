// Mutual exclusion spin locks. 
// 旋轉鎖，透過禁止中斷與 hartid 檢查，避免兩個核心同時存取某變數的機制

// Mutual exclusion lock. (旋轉鎖，透過禁止中斷與 hartid 檢查，避免兩個核心同時存取某變數的機制)
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

void lock_init(struct spinlock *lk, char *name) // 設定一個鎖
{
  lk->name = name; // 鎖的名稱
  lk->locked = 0;  // 初始值為未上鎖
  lk->cpu = 0;     // 哪個處理器鎖的？
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void lock_acquire(struct spinlock *lk) {
}

// Release the lock.
void lock_release(struct spinlock *lk) {
}

int lock_holding(struct sleeplock *lk) // 檢查本行程是否鎖定了 lk
{
  int r;
  
  lock_acquire(&lk->lk);
  // r = lk->locked && (lk->pid == myproc()->pid);
  r = lk->locked;
  lock_release(&lk->lk);
  return r;
}
