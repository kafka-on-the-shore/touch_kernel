/*
 * fork exmaples
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

/* wai till all son proces ends */
void waitall(void)
{
    pid_t pid;
    int status;
    while (1) {
        /* block until the son process ends */
        pid = wait(&status);
        printf("--> Son process %d ends now\n", pid);
        if (pid == -1) {
            /* no other son process now */
            if (errno == ECHILD)
                break;
            perror("wait");
            exit(-1);
        }
    }
}

int main()
{
    pid_t pid;
    /* forked processes do not share memory, here a_var */
    int a_var;

    pid = fork();
    if (pid == 0) {
        /* child */
        a_var = 0;
        printf("%d: hello, I'm child\n", a_var);
    } else if (pid < 0) {
        /* parent, upon error */
        perror("fork failed");
        return -1;
    } else {
        /* parent, fork suceed and go on */
        a_var = pid;
        waitall();
        printf("%d: hello, I'm papa\n", a_var);
    }
    return 0;
}

