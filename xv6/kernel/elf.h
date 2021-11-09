// Format of an ELF executable file
// 參考 -- http://ccckmit.wikidot.com/lk:elf

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  uint magic;       // must equal ELF_MAGIC (ELF 魔數，一個特殊的辨識代號)
  uchar elf[12];    // ELF 辨識代號區
  ushort type;      // 檔案類型代號
  ushort machine;   // 機器平台代號
  uint version;     // 版本資訊
  uint64 entry;     // 程式的起始位址
  uint64 phoff;     // 程式表頭的位址
  uint64 shoff;     // 分段表頭的位址
  uint flags;       // 與處理器有關的旗標值
  ushort ehsize;    // ELF檔頭的長度
  ushort phentsize; // 程式表頭的記錄長度
  ushort phnum;     // 程式表頭的記錄個數
  ushort shentsize; // 分段表頭的記錄長度
  ushort shnum;     // 分段表頭的記錄個數
  ushort shstrndx;  // 分段字串表 .shstrtab 的分段代號
};

// Program section header
struct proghdr {
  uint32 type;   // 分區類型
  uint32 flags;  // 分區旗標 (READ/WRITE/EXEC)
  uint64 off;    // 分區位址 (在目的檔中)
  uint64 vaddr;  // 分區的虛擬記憶體位址
  uint64 paddr;  // 分區的實體記憶體位址
  uint64 filesz; // 分區在檔案中的大小
  uint64 memsz;  // 分區在記憶體中的大小
  uint64 align;  // 分區的對齊資訊
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
