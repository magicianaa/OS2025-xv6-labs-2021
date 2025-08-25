// user/trace.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int mask;
  char *cmdargv[argc];
  
  if (argc < 2) {
    fprintf(2, "usage: trace mask command [args...]\n");
    exit(1);
  }
  
  // 解析掩码参数
  mask = atoi(argv[1]);
  if (trace(mask) < 0) {
    fprintf(2, "trace: failed to set mask\n");
    exit(1);
  }
  
  // 构造命令参数
  for (int i = 2; i < argc; i++) {
    cmdargv[i-2] = argv[i];
  }
  cmdargv[argc-2] = 0;
  
  // 执行命令
  exec(cmdargv[0], cmdargv);
  fprintf(2, "trace: exec failed\n");
  exit(1);
}