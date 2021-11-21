FILE *fd;

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
// 磁碟布局
// [ boot 區塊  | 超級區塊 | 日誌 | inode 區塊   | free bit map | 資料區塊     ]

void disk_fs_create(char *imgFile, int nblock) {
	FILE *f = fopen(imgFile, "w+");
  byte block[BSIZE];
  fwrite(block, BSIZE, nblock, f);
  fclose(f);
}

void disk_rw(struct buf *b, int write)
{
  uint sector = b->blockno * (BSIZE / 512);

  acquire(&disk.vdisk_lock);

  if (write) {
      fwrite(b->buf, BSIZE, 1, fd);
  } else {
      fread(b->buf, BSIZE, 1, fd);
  }

  release(&disk.vdisk_lock);
}