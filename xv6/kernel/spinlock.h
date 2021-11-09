// Mutual exclusion lock. (旋轉鎖，透過禁止中斷與 hartid 檢查，避免兩個核心同時存取某變數的機制)
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

