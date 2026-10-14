/*
 * 2_pipeline.c — ":" 로 구분된 두 명령을 이어 붙여 파이프로 실행한다
 *
 * [핵심 개념]
 *   dup2(a, b) 는 디스크립터 b 가 a 와 같은 곳을 가리키게 만든다.
 *   자식1의 표준출력(1)을 파이프 쓰기 끝으로, 자식2의 표준입력(0)을 파이프 읽기 끝으로 교체한다.
 *   부모는 파이프 양쪽을 모두 닫아야 뒤쪽 명령어가 EOF를 보고 정상 종료된다.
 *   출처: man 2 dup2, man 2 pipe, man 3 execvp
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_pipeline 2_pipeline.c
 *   ./2_pipeline ls -l : wc -l
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    /* 1. ":" 구분자 위치 찾기 */
    int split_idx = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) {
            split_idx = i;
            break;
        }
    }

    /* 구분자가 없거나 맨 앞/맨 뒤에 위치하면 사용법 출력 후 종료 */
    if (split_idx <= 1 || split_idx >= argc - 1) {
        fprintf(stderr, "사용법: %s <명령1> [옵션...] : <명령2> [옵션...]\n", argv[0]);
        return 1;
    }

    /* ":" 자리를 NULL로 만들어 cmd1과 cmd2 인자 배열로 분리 */
    argv[split_idx] = NULL;
    char **cmd1 = &argv[1];
    char **cmd2 = &argv[split_idx + 1];

    int fd[2];   /* fd[0] = 읽기 끝, fd[1] = 쓰기 끝 */
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    /* 첫 번째 자식: 앞 명령(cmd1)을 실행하고 출력을 파이프로 보낸다 */
    if (fork() == 0) {
        close(fd[0]);                 /* 읽기 끝은 쓰지 않는다 */
        dup2(fd[1], STDOUT_FILENO);   /* 표준출력을 파이프 쓰기 끝으로 교체 */
        close(fd[1]);                 /* 원본 디스크립터 닫기 */
        execvp(cmd1[0], cmd1);        /* 분리한 앞쪽 명령어 실행 */
        perror("execvp cmd1");
        _exit(127);
    }

    /* 두 번째 자식: 뒤 명령(cmd2)을 실행하고 입력을 파이프에서 받는다 */
    if (fork() == 0) {
        close(fd[1]);                 /* 쓰기 끝은 쓰지 않는다 */
        dup2(fd[0], STDIN_FILENO);    /* 표준입력을 파이프 읽기 끝으로 교체 */
        close(fd[0]);                 /* 원본 디스크립터 닫기 */
        execvp(cmd2[0], cmd2);        /* 분리한 뒤쪽 명령어 실행 */
        perror("execvp cmd2");
        _exit(127);
    }

    /* 부모는 양쪽 다 닫아야 한다. 안 닫으면 뒤 명령이 EOF를 못 봐서 끝나지 않는다. */
    close(fd[0]);
    close(fd[1]);

    wait(NULL);   /* 자식 1 종료 대기 */
    wait(NULL);   /* 자식 2 종료 대기 */
    return 0;
}