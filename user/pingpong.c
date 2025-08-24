#include "user/user.h"

int main() {
    int p1[2], p2[2];  // 两个管道：p1(父→子)，p2(子→父)
    char buf[1];       // 用于传递的消息（单个字节）

    // 创建管道
    if (pipe(p1) < 0 || pipe(p2) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();  // 创建子进程
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {  // 子进程
        close(p1[1]);  // 关闭p1的写端（子进程只读）
        close(p2[0]);  // 关闭p2的读端（子进程只写）

        // 从p1读取父进程消息
        if (read(p1[0], buf, 1) != 1) {
            fprintf(2, "child read failed\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());  // 输出提示

        // 向p2写入消息给父进程
        if (write(p2[1], buf, 1) != 1) {
            fprintf(2, "child write failed\n");
            exit(1);
        }

        close(p1[0]);
        close(p2[1]);
        exit(0);
    } else {  // 父进程
        close(p1[0]);  // 关闭p1的读端（父进程只写）
        close(p2[1]);  // 关闭p2的写端（父进程只读）

        // 向p1写入消息给子进程
        if (write(p1[1], "x", 1) != 1) {
            fprintf(2, "parent write failed\n");
            exit(1);
        }

        // 从p2读取子进程消息
        if (read(p2[0], buf, 1) != 1) {
            fprintf(2, "parent read failed\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());  // 输出提示

        close(p1[1]);
        close(p2[0]);
        wait(0);  // 等待子进程结束
        exit(0);
    }
}