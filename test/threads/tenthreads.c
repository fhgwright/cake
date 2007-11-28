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

    return NULL;
}

int main (int argc, char **argv) {
    int i;
    ThreadIdentifier id;

    for (i = 0; i < 10; i++) {
        id = CreateThread(thread_main, NULL);
        printf("created thread %d\n", id);
        Delay(100);
    }

    return 0;
}
