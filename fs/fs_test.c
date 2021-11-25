#include <stdio.h>
#include "fs.h"

uint open(char *path);

int main() {
    uint fd = open("README.md");
    printf("fd=%d\n", fd);
}

uint open(char *path, int omode) {
  int fd;
  struct file *f;
  struct inode *ip;

  begin_op();

  if(omode & O_CREATE){ // 模式為創建新檔案
  /*
    ip = create(path, T_FILE, 0, 0); // 創建新的 inode
    if(ip == 0){
      end_op();
      return -1;
    }
  */
  } else {
    if((ip = namei(path)) == 0){ // 取得該路徑名對應的 inode
      end_op();
      return -1;
    }
    ilock(ip); // 將該 inode 鎖定
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip); // 若是目錄且要寫入，那就先解鎖該 inode
      end_op();
      return -1;
    }
  }

  if(ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)){
    iunlockput(ip); // 若是裝置，那也先解鎖該 inode
    end_op();
    return -1;
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){ // 分配檔案描述子
    if(f) // 若分配失敗就關閉之。
      fileclose(f);
    iunlockput(ip); // 分配成功的話，先解鎖該 inode
    end_op();
    return -1;
  }
  // 設定 FILE (struct file) 結構 (也就是檔案描述子)
  if(ip->type == T_DEVICE){
    f->type = FD_DEVICE;
    f->major = ip->major;
  } else {
    f->type = FD_INODE;
    f->off = 0;
  }
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if((omode & O_TRUNC) && ip->type == T_FILE){
    itrunc(ip); // 若是檔案且模式為 O_TRUNC，就將該 inode 清空。
  }

  iunlock(ip);
  end_op();

  return fd; // 傳回檔案描述子代碼 (在檔案表 ftable 中的位置)
}
    