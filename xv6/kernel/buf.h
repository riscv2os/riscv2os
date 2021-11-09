struct buf { // 緩衝區塊
  int valid;    // has data been read from disk? 已讀入？
  int disk;     // does disk "own" buf?
  uint dev;     // 裝置代號
  uint blockno; // 區塊代號
  struct sleeplock lock; // 鎖定中？
  uint refcnt;      // 引用數量
  struct buf *prev; // LRU cache list
  struct buf *next; // 緩衝區塊串列
  uchar data[BSIZE]; // 資料內容
};

