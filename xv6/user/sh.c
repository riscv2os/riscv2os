// Shell. (指令列解譯器)

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Parsed command representation
#define EXEC  1 // 單一指令        例如: cat README
#define REDIR 2 // IO 轉向型指令   例如: cat README > temp.txt
#define PIPE  3 // 管線型指令      例如: cat README | grep xv6
#define LIST  4 // 列表型指令      例如: cat README ; ls
#define BACK  5 // 背景執行        例如: cat README&

#define MAXARGS 10

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns. (執行指令，不會返回，因為是 fork 的，最後用 exit 離開)
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0) // 若是空行，直接離開
    exit(1);

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC: // 若是單一指令  例如: cat README
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(1);
    exec(ecmd->argv[0], ecmd->argv); // 將 fork 的行程替換成該指令去執行
    fprintf(2, "exec %s failed\n", ecmd->argv[0]); // 若 exec() 返回代表失敗，則印出錯誤訊息
    break;

  case REDIR: // 若是 IO 重新導向指令 (導入檔案)  例如: cat README > temp.txt
    rcmd = (struct redircmd*)cmd; 
    close(rcmd->fd); // 關閉原本的 fd (通常是 1 : console)
    if(open(rcmd->file, rcmd->mode) < 0){ // 開啟要導入的檔案 (此時會取代 rcmd->fd)
      fprintf(2, "open %s failed\n", rcmd->file);
      exit(1);
    }
    runcmd(rcmd->cmd); // 執行該指令
    break;

  case LIST: // 若是串列型指令  例如: cat README ; ls
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left); // 先執行最左邊指令 (第一個指令)
    wait(0);
    runcmd(lcmd->right); // 再遞迴執行剩下的 (右半部)
    break;

  case PIPE: // 若是管線型指令  例如: cat README | grep xv6
    // cmd1 | cmd2 | cmd3 ....   (left=cmd1, right=cmd2|cmd3...)
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0) // 創建管道 p
      panic("pipe");
    if(fork1() == 0){ // fork 出第一個新行程 (left=cmd1)
      close(1); // 關閉 fd=1 (stdout) 
      dup(p[1]); // 用 p[1] 取代 fd=1 (將輸出重新導向)   => cmd1 的輸出要導入 cmd2...
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left); // 執行最左邊指令 (第一個指令)
    }
    if(fork1() == 0){ // fork 出第二個新行程 (right=cmd2...)
      close(0);   // 關閉 fd=0 (stdin)
      dup(p[0]);  // 用 p[0] 取代 fd=0 (將輸入重新導向)   => cmd2 的輸入是接收前面傳來的 ...
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right); // 遞迴執行剩下的 (右半部)
    }
    // 關閉管道
    close(p[0]);
    close(p[1]);
    wait(0); // 等待 fork 的行程執行完 (fork1 兩次，所以要等兩次)
    wait(0);
    break;

  case BACK: // 背景執行  例如: cat README&
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0) // 第二次 fork 新的子行程
      runcmd(bcmd->cmd); // 用子行程去執行背景指令。
    // (原本 fork 的行程會被 wait，而第二次 fork 者不會被 wait，所以就變成在背景執行了)
    break;
  }
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  fprintf(2, "$ ");     // 印出 $ 提示
  memset(buf, 0, nbuf); // 清空 buf
  gets(buf, nbuf);      // 讀入一行指令到 buf 中
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void) // shell 主程式
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){ // 確保 console 存在且可讀寫
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){ // 取得下一列命令
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){ // 如果是 cd 指令
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0) // 切換到 cd 指定的資料夾
        fprintf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0) // 否則呼叫 fork1() 分支出新的子行程 
      runcmd(parsecmd(buf)); // 讓子行程去執行該段命令
    wait(0);
  }
  exit(0);
}

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s) // 解析指令
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es) // 例如： cat README | grep xv6 & ; ls
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es); // 取得一段 pipe 型指令
  while(peek(ps, es, "&")){ // 若後面跟著 &，則是背景型指令
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd); // 創建背景型指令結構
  }
  if(peek(ps, es, ";")){ // 若後面跟著 ; 則是串列型指令
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es)); // 創建串列型指令結構
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE|O_TRUNC, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
