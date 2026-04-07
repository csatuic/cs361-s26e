#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define ITERATIONS 1000000000LL

int counter;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* thread_function(void* arg)
{
    pthread_mutex_lock(&lock);
    counter++;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    pthread_t thread1;
    
    /* Create the two worker threads. */
    if (pthread_create(&thread1, NULL, thread_function, NULL) != 0) {
        fprintf(stderr, "Thread creation failed.\n");
        return EXIT_FAILURE;
    }
    pthread_mutex_lock(&lock);
    printf("Counter is %d\n",counter);
    pthread_mutex_unlock(&lock);

    /* Main thread waits for both workers to complete. */
    pthread_join(thread1, NULL);

    return EXIT_SUCCESS;
}
