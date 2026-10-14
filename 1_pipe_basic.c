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

int main(void)
{
    int fd[2];   /* fd[0] = 읽기 끝, fd[1] = 쓰기 끝이 여기 채워진다 */
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();   /* fork 이후 부모·자식이 fd[0], fd[1] 을 그대로 물려받아 같이 갖는다 */
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        /* 자식 = 읽는 쪽. 쓰기 끝을 닫는다. */
        close(fd[1]);

        char buf[64];
        /* 파이프에서 읽는다. 마지막 한 자리는 남겨 뒤에 '\0'을 붙일 자리로 쓴다 */
        ssize_t n = read(fd[0], buf, sizeof buf - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("  [자식] 받은 내용: %s\n", buf);
        }
        close(fd[0]);
        fflush(stdout);   /* _exit 은 버퍼를 비우지 않는다 — 직접 비워야 출력이 남는다 */
        _exit(0);
    }

    /* 부모 = 쓰는 쪽. 읽기 끝을 닫는다. */
    close(fd[0]);

    const char *msg = "안녕, 자식 프로세스!";
    write(fd[1], msg, strlen(msg));   /* 널 종료 문자는 보내지 않는다 — 길이만큼만 씀 */
    close(fd[1]);   /* 닫아야 자식의 read 가 끝난다 */

    wait(NULL);
    printf("[부모] 전송 완료\n");
    return 0;
}
