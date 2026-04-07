#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define ITERATIONS 1000000000LL

long long counter;
pthread_barrier_t barrier;

void* thread_function(void* arg)
{
    /* Both threads wait at the barrier to ensure synchronized start of the increment loop. */
    (void)pthread_barrier_wait(&barrier);

    /* Each thread performs one billion atomic increments using C11 atomics. */
    for (long long i = 0; i < ITERATIONS; ++i) {
	counter++;
    }

    return NULL;
}

int main(void)
{
    pthread_t thread1, thread2;

    /* Initialize the atomic counter to zero (C11 atomics). */
    atomic_init(&counter, 0);

    /* Initialize the barrier for exactly two threads (POSIX threads are used here
     * because the C11 <threads.h> library does not provide a barrier primitive). */
    if (pthread_barrier_init(&barrier, NULL, 2) != 0) {
        fprintf(stderr, "Barrier initialization failed.\n");
        return EXIT_FAILURE;
    }

    /* Create the two worker threads. */
    if (pthread_create(&thread1, NULL, thread_function, NULL) != 0 ||
        pthread_create(&thread2, NULL, thread_function, NULL) != 0) {
        fprintf(stderr, "Thread creation failed.\n");
        pthread_barrier_destroy(&barrier);
        return EXIT_FAILURE;
    }

    /* Main thread waits for both workers to complete. */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    /* Clean up the barrier. */
    pthread_barrier_destroy(&barrier);

    /* Print the final value of the atomic counter (must be exactly 2000000000). */
    printf("%lld\n", counter);

    return EXIT_SUCCESS;
}
