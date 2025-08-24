#include "user/user.h"

// 子进程处理函数：从管道读取数据，筛选素数并传递给下一个进程
void primes(int p_read) {
    int first, num;
    // 读取第一个数（当前管道中的最小数，必为素数）
    if (read(p_read, &first, sizeof(int)) != sizeof(int)) {
        close(p_read);
        exit(0);
    }
    printf("prime %d\n", first);  // 输出素数

    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {  // 子进程：继续筛选
        close(p[1]);  // 关闭写端
        primes(p[0]);  // 递归处理
    } else {  // 父进程：过滤数据并传递给子进程
        close(p[0]);  // 关闭读端
        // 读取剩余数字，只传递不能被first整除的数
        while (read(p_read, &num, sizeof(int)) == sizeof(int)) {
            if (num % first != 0) {
                write(p[1], &num, sizeof(int));
            }
        }
        close(p_read);
        close(p[1]);
        wait(0);  // 等待子进程结束
        exit(0);
    }
}

int main() {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {  // 子进程：启动筛选
        close(p[1]);
        primes(p[0]);
    } else {  // 父进程：向管道写入2-35的数字
        close(p[0]);
        for (int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
        wait(0);  // 等待所有子进程结束
        exit(0);
    }
    return 0;
}