struct file { // 檔案描述子
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type; // 檔案類型
  int ref; // reference count (引用次數)
  char readable;  // 可讀
  char writable;  // 可寫
  struct pipe *pipe; // FD_PIPE  (管線)
  struct inode *ip;  // FD_INODE and FD_DEVICE (inode 指標)
  uint off;          // FD_INODE (目前位移)
  short major;       // FD_DEVICE (主序號)
};

#define major(dev)  ((dev) >> 16 & 0xFFFF) // 取得裝置主序號
#define minor(dev)  ((dev) & 0xFFFF)       // 取得裝置次序號
#define	mkdev(m,n)  ((uint)((m)<<16| (n))) // 主序號+次序號=完整序號

// in-memory copy of an inode (記憶體內的 inode)
struct inode {
  uint dev;           // Device number (裝置號碼)
  uint inum;          // Inode number (inode 號碼)
  int ref;            // Reference count (引用次數)
  struct sleeplock lock; // protects everything below here (鎖)
  int valid;          // inode has been read from disk? (是否讀進來了)

  short type;         // copy of disk inode (inode 型態)
  short major;        // 主序號
  short minor;        // 次序號
  short nlink;        // 連結次數
  uint size;          // 檔案大小
  uint addrs[NDIRECT+1]; // 區塊位址
};

// map major device number to device functions. (將主裝置映射到 read/write 函數)
struct devsw {
  int (*read)(int, uint64, int);  // 裝置讀取函數
  int (*write)(int, uint64, int); // 裝置寫入函數
};

extern struct devsw devsw[]; // 裝置列表

#define CONSOLE 1 // 控制台代號
