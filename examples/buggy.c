#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define ITERATIONS 1000000000LL

int counter;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

void* thread_function(void* arg)
{
    pthread_mutex_lock(&lock);
    counter++;
    pthread_mutex_unlock(&lock);

    pthread_barrier_wait(&barrier);

    // wait for main to finish printing before decrementing
    pthread_barrier_wait(&barrier);

    pthread_mutex_lock(&lock);
    counter--;
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void)
{
    pthread_t thread1;

    pthread_barrier_init(&barrier,0,2);
    if (pthread_create(&thread1, NULL, thread_function, NULL) != 0) {
        fprintf(stderr, "Thread creation failed.\n");
        return EXIT_FAILURE;
    }

    pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&lock);
    printf("Counter is %d\n",counter);    
    pthread_mutex_unlock(&lock);

    pthread_barrier_wait(&barrier);


    pthread_join(thread1, NULL);


    return EXIT_SUCCESS;
}
