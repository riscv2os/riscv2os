// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO  1   // root i-number
#define BSIZE 1024  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock { // 超級區塊
  uint magic;        // Must be FSMAGIC      // 用來辨識的魔數:0x10203040
  uint size;         // Size of file system image (blocks) // 全部區塊數
  uint nblocks;      // Number of data blocks // 資料區塊數
  uint ninodes;      // Number of inodes.     // inodes 數量
  uint nlog;         // Number of log blocks  // 日誌 log 的區塊數
  uint logstart;     // Block number of first log block   // log 的首區塊
  uint inodestart;   // Block number of first inode block // inode 的首區塊
  uint bmapstart;    // Block number of first free map block // free bitmap 的首區塊
};

#define FSMAGIC 0x10203040

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure (硬碟中的 inode 結構)
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block. (每個區塊的 inode 數量)
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i (第 i 個 inode 存放在哪個區塊)
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block (每個區塊包含幾個 bits)
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b (取得 free map 中紀錄該區塊是否 free 的區塊代號)
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

