void disk_rw(struct buf *b, int write)
{
  uint sector = b->blockno * (BSIZE / 512);

  acquire(&disk.vdisk_lock);

  if (write) {
      ffs_wsect(sector, b->buf);
  } else {
      ffs_rsect(sector, b->buf);
  }

  release(&disk.vdisk_lock);
}