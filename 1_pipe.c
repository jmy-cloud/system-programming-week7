/*
 * 1_pipe_basic.c — 파이프로 부모가 자식에게 문자열을 보낸다
 *
 * [핵심 개념]
 *   pipe(2) 는 파일 디스크립터 두 개를 준다: fd[0] 은 읽기, fd[1] 은 쓰기.
 *   fork 하면 자식이 이 디스크립터를 물려받으므로 부모-자식이 통신할 수 있다.
 *   각자 쓰지 않는 쪽은 반드시 닫아야 한다 — 쓰기 쪽이 모두 닫혀야 읽기 쪽이 EOF(0)를 본다.
 *   출처: man 2 pipe, man 7 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_pipe_basic 1_pipe_basic.c
 *   ./1_pipe_basic
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "사용법: %s <문자열>\n", argv[0]);
        return 1;
    }
    const char *msg = argv[1];

    int p2c[2];   /* 부모 -> 자식 */
    int c2p[2];   /* 자식 -> 부모 */
    if (pipe(p2c) == -1) { perror("pipe"); return 1; }
    if (pipe(c2p) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        /* 자식 = p2c에서 읽고, c2p로 쓴다 */
        close(p2c[1]);
        close(c2p[0]);

        char buf[64];
        ssize_t n = read(p2c[0], buf, sizeof buf - 1);
        if (n > 0) {
            buf[n] = '\0';
            /* 소문자를 대문자로 직접 변환 ('a'~'z' -> 'A'~'Z') */
            for (int i = 0; i < n; i++) {
                if (buf[i] >= 'a' && buf[i] <= 'z') {
                    buf[i] = buf[i] - 'a' + 'A';
                }
            }
            write(c2p[1], buf, n);
        }
        close(p2c[0]);
        close(c2p[1]);
        fflush(stdout);
        _exit(0);
    }

    /* 부모 = p2c로 쓰고, c2p에서 읽는다 */
    close(p2c[0]);
    close(c2p[1]);

    write(p2c[1], msg, strlen(msg));
    close(p2c[1]);   /* 닫아야 자식의 read 가 끝난다 */

    char res[64];
    ssize_t n = read(c2p[0], res, sizeof res - 1);
    if (n > 0) {
        res[n] = '\0';
        printf("[부모] 자식이 대문자로 바꾼 내용: %s\n", res);
    }
    close(c2p[0]);

    wait(NULL);
    return 0;
}
