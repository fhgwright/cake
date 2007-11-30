#include <libraries/thread.h>
#include <proto/thread.h>
#include <proto/dos.h>
#include <stdio.h>

void *thread_main(void *data) {
    ThreadIdentifier id = CurrentThread();
    int i;

    printf("[%d] starting\n", id);

    for (i = 0; i < 10; i++) {
        printf("[%d] count: %d\n", id, i);
        Delay(25);
    }

    printf("[%d] exiting\n", id);

    return (void *) id;
}

int main (int argc, char **argv) {
    int i;
    ThreadIdentifier id[10], ret;

    for (i = 0; i < 10; i++) {
        id[i] = CreateThread(thread_main, NULL);
        printf("created thread %d\n", id[i]);
        Delay(100);
    }

    for (i = 0; i < 10; i++) {
        printf("waiting for thread %d\n", id[i]);
        WaitForThreadCompletion(id[i], (void **) &ret);
        printf("thread %d return %d\n", id[i], ret);
    }

    return 0;
}
