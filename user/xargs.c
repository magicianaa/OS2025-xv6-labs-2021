#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAXARGS 10  // 最大参数数量（参考sh.c）
#define BUFSIZE 512 // 输入缓冲区大小

// 从标准输入读取所有参数（按换行分割）
// 存储到args中，返回参数数量
int read_args(char *args[]) {
    char buf[BUFSIZE];
    int n;
    int arg_idx = 0;
    char *p = buf;

    while ((n = read(0, p, 1)) > 0) {
        if (*p == '\n' || *p == ' ') { // 按换行或空格分割参数
            *p = '\0'; // 终止当前参数
            if (p > buf) { // 忽略空行/空参数
                if (arg_idx >= MAXARGS) {
                    fprintf(2, "xargs: too many arguments\n");
                    exit(1);
                }
                // 手动分配内存并复制字符串（替代strdup）
                char *arg = malloc(strlen(buf) + 1);
                if (arg == 0) {
                    fprintf(2, "xargs: out of memory\n");
                    exit(1);
                }
                strcpy(arg, buf);
                args[arg_idx++] = arg;
            }
            p = buf; // 重置缓冲区指针
        } else {
            p++;
            if (p - buf >= BUFSIZE - 1) { // 防止缓冲区溢出
                fprintf(2, "xargs: argument too long\n");
                exit(1);
            }
        }
    }

    // 处理最后一个未被换行终止的参数
    if (p > buf) {
        *p = '\0';
        if (arg_idx < MAXARGS) {
            // 手动分配内存并复制字符串（替代strdup）
            char *arg = malloc(strlen(buf) + 1);
            if (arg == 0) {
                fprintf(2, "xargs: out of memory\n");
                exit(1);
            }
            strcpy(arg, buf);
            args[arg_idx++] = arg;
        } else {
            fprintf(2, "xargs: too many arguments\n");
            exit(1);
        }
    }

    return arg_idx;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "usage: xargs <command> [args...]\n");
        exit(1);
    }

    // 存储从标准输入读取的参数
    char *input_args[MAXARGS];
    int input_argc = read_args(input_args);

    // 组合命令参数：xargs后的命令部分 + 输入参数
    char *cmd_args[MAXARGS];
    int cmd_argc = 0;

    // 复制xargs后的基础命令（如"grep", "hello"）
    for (int i = 1; i < argc; i++) {
        if (cmd_argc >= MAXARGS) {
            fprintf(2, "xargs: command too long\n");
            exit(1);
        }
        cmd_args[cmd_argc++] = argv[i];
    }

    // 追加输入参数（如从find得到的路径）
    for (int i = 0; i < input_argc; i++) {
        if (cmd_argc >= MAXARGS - 1) { // 留一个位置给NULL
            fprintf(2, "xargs: total arguments too long\n");
            exit(1);
        }
        cmd_args[cmd_argc++] = input_args[i];
    }
    cmd_args[cmd_argc] = 0; // 终止参数列表

    // 执行组合后的命令
    if (fork() == 0) {
        exec(cmd_args[0], cmd_args);
        fprintf(2, "xargs: exec %s failed\n", cmd_args[0]);
        exit(1);
    }

    // 等待子进程完成
    wait(0);

    // 释放动态分配的输入参数
    for (int i = 0; i < input_argc; i++) {
        free(input_args[i]);
    }

    exit(0);
}