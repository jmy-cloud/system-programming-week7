/*
 * 3_fifo_read.c — FIFO(이름 있는 파이프)로 메시지를 받는다 [먼저 실행할 쪽]
 *
 * [핵심 개념]
 *   FIFO 는 파일 시스템에 이름을 가지므로 관계없는 두 프로그램도 통신할 수 있다.
 *   open 은 상대가 반대쪽을 열어 줄 때까지 기다린다(블록).
 *   다 쓴 FIFO 는 unlink() 로 지워야 파일 시스템에 남지 않는다.
 *   출처: man 3 mkfifo, man 7 fifo, man 2 unlink
 *
 * [컴파일·실행] 터미널 두 개를 쓴다
 *   gcc -Wall -Wextra -o 3_fifo_read 3_fifo_read.c
 *   ./3_fifo_read          # 먼저 실행 (대기)
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(void)
{
    /* FIFO 를 만든다. 이미 있으면(EEXIST) 그대로 쓴다. */
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("보내는 쪽을 기다립니다... (%s)\n", FIFO_PATH);

    /* 상대가 열 때까지 여기서 기다린다 (블록) */
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        unlink(FIFO_PATH);
        return 1;
    }

    char buf[128];
    /* 마지막 한 자리는 남겨 두어 뒤에 '\0'을 붙일 자리로 쓴다 */
    ssize_t n = read(fd, buf, sizeof buf - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("받은 메시지: %s\n", buf);
    } else if (n == -1) {
        perror("read");
    }

    close(fd);

    /* 다 썼으면 FIFO 파일도 지운다 — 안 지우면 파일시스템에 계속 남는다 */
    if (unlink(FIFO_PATH) == -1) {
        perror("unlink");
        return 1;
    }
    printf("FIFO 정리 완료: %s 삭제됨\n", FIFO_PATH);

    return 0;
}