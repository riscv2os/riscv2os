// init: The initial user-level program (第一個使用者行程，會呼叫 sh)

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){ // 開啟 console 裝置
    mknod("console", CONSOLE, 0); // 若開啟失敗，則創建該裝置為第 0 號檔案 (stdin)。
    open("console", O_RDWR);
  }
  dup(0);  // stdout // 複製 stdin 到 stdout
  dup(0);  // stderr // 複製 stdin 到 stderr

  for(;;){
    printf("init: starting sh\n");
    pid = fork(); // fork 出一個子行程
    if(pid < 0){
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv); // 將該子行程替換為 sh (shell)
      printf("init: exec sh failed\n");
      exit(1);
    }

    for(;;){ // 等候 shell 行程退出，之後會再啟動另一個 shell
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *) 0);
      if(wpid == pid){
        // the shell exited; restart it.
        break;
      } else if(wpid < 0){
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
    }
  }
}
