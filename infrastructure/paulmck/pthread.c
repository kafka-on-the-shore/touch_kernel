#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* thread share the value, which is different with fork */
static int g_var = 0;

void *mythread(void *arg)
{
    g_var = 1;
    printf("Child process set g_var = 1\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    void *vp;

    if (pthread_create(&tid, NULL, mythread, NULL) != 0) {
        perror("pthread create failed");
        exit(-1);
    }

    /* pthread_join works like wait() in fork, blocked till tid over */
    /* vp is the return value of tid */
    if (pthread_join(tid, &vp) != 0) {
        perror("pthread join failed");
        exit(-1);
    }
    printf("Parent process sees g_var=%d\n", g_var);
    return 0;
}

