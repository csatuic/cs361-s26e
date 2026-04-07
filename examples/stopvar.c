#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

atomic_bool stop=0;
//volatile int stop=0;

void* thread_function(void* arg)
{
    while(!atomic_load(&stop)) {
    };
    return NULL;
}

int main(void)
{
    pthread_t thread1;

    if (pthread_create(&thread1, NULL, thread_function, NULL) != 0) {
        fprintf(stderr, "Thread creation failed.\n");
        return EXIT_FAILURE;
    }

    sleep(1);
    atomic_store(&stop,1);

    /* Main thread waits for both workers to complete. */
    pthread_join(thread1, NULL);

    return EXIT_SUCCESS;
}
