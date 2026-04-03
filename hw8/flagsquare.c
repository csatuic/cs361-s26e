#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

#define NUM_FLAGS 150
#define ROWS 10
#define COLS 15
#define QUEUE_SIZE 100
#define DEFAULT_NUM_JOBS 100000
#define DISPLAY_US 250000   // 250 ms refresh

enum JobType {
    JOB_ADD,
    JOB_REMOVE,
    JOB_SWAP,
    JOB_EXIT,
};

struct Job {
    int id;
    enum JobType type;
    int idx1;
    int idx2;   // only used for SWAP
};

typedef struct {
    struct Job jobs[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    bool done;
} WorkQueue;

WorkQueue queue;
int flags[NUM_FLAGS];
int flag_count;
pthread_mutex_t count_mutex;
pthread_mutex_t flag_mutexes[NUM_FLAGS];

void init_queue() {
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    queue.done = false;

    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.not_full, NULL);
    pthread_cond_init(&queue.not_empty, NULL);
}

void destroy_queue() {
    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.not_full);
    pthread_cond_destroy(&queue.not_empty);
}

void enqueue(struct Job job) {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count == QUEUE_SIZE && !queue.done) {
        pthread_cond_wait(&queue.not_full, &queue.mutex);
    }
    if (queue.done) {
        return;
    }
    queue.jobs[queue.tail] = job;
    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
    queue.count++;
    pthread_cond_broadcast(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
}

struct Job dequeue() {
    struct Job job;
    pthread_mutex_lock(&queue.mutex);
    while (queue.count == 0 && !queue.done) {
        pthread_cond_wait(&queue.not_empty, &queue.mutex);
    }
    if (queue.count == 0 && queue.done) {
        job.type=JOB_EXIT;
        pthread_mutex_unlock(&queue.mutex);
        return job;
    }
    job = queue.jobs[queue.head];
    queue.head = (queue.head + 1) % QUEUE_SIZE;
    queue.count--;
    pthread_cond_signal(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
    return job;
}

void init_flags() {
    for (int i = 0; i < NUM_FLAGS; i++) {
        flags[i] = (i % 2 == 0) ? 1 : 0;
        pthread_mutex_init(&flag_mutexes[i], NULL);
    }
    flag_count = NUM_FLAGS / 2;  
}

void destroy_mutexes() {
    for (int i = 0; i < NUM_FLAGS; i++) {
        pthread_mutex_destroy(&flag_mutexes[i]);
    }
    pthread_mutex_destroy(&count_mutex);
}

void perform_job(struct Job *job) {
    if (job->type == JOB_SWAP) {
        int i1 = job->idx1;
        int i2 = job->idx2;

        pthread_mutex_lock(&flag_mutexes[i1]);
        pthread_mutex_lock(&flag_mutexes[i2]);

        if (flags[i1] == 1 && flags[i2] == 0) {
            flags[i1] = 0;
            flags[i2] = 1;
        } else if (flags[i1] == 0 && flags[i2] == 1) {
            flags[i1] = 1;
            flags[i2] = 0;
        }
        // count unchanged

        pthread_mutex_unlock(&flag_mutexes[i2]);
        pthread_mutex_unlock(&flag_mutexes[i1]);
    } 
    else {  // ADD or REMOVE
        int idx = job->idx1;
        pthread_mutex_lock(&flag_mutexes[idx]);

        if (job->type == JOB_ADD) {
            if (flags[idx] == 0) {
                flags[idx] = 1;
                flag_count++;
            }
        } 
        else if (job->type == JOB_REMOVE) {
            if (flags[idx] == 1) {
                flags[idx] = 0;
                flag_count--;
            }
        }
        pthread_mutex_unlock(&flag_mutexes[idx]);
    }
}

void print_grid() {
    int global_count;
    pthread_mutex_lock(&count_mutex);
    global_count = flag_count;
    pthread_mutex_unlock(&count_mutex);

    int actual_count = 0;
    for (int i = 0; i < NUM_FLAGS; i++) {
        actual_count += flags[i];
    }

    printf("\033[2J\033[H");  // clear screen and home cursor
    printf("Global Count: %d    Actual Count: %d\n\n", global_count, actual_count);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int idx = r * COLS + c;
            printf("%s ", flags[idx] ? "x" : ".");
        }
        printf("\n");
    }
    fflush(stdout);
}

void* display_thread(void* arg) {
    (void)arg;
    while (1) {
        print_grid();
        usleep(DISPLAY_US);
    }
    return NULL;
}

void* worker_thread(void* arg) {
    while (1) {
        struct Job job = dequeue();
        if (job.type==JOB_EXIT) {
            break;
        }
        perform_job(&job);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int num_workers = 0;
    int delay = 0;
    int quiet = 0;
    int opt;
    int num_jobs = DEFAULT_NUM_JOBS;
    bool enable_swap = false;
    while ((opt = getopt(argc, argv, "w:j:shd:q")) != -1) {
        switch (opt) {
            case 'w':
                num_workers = atoi(optarg);
                break;
            case 'j':
                num_jobs = atoi(optarg);
                break;
            case 's':
                enable_swap = true;
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'q':
                quiet = true;
                break;
            case 'h':
            default:
                printf("Usage: %s -w <num_workers> [-j <num_jobs>] [-s] [-d <usec_delay>] [-q]\n", argv[0]);
                printf("  -s : enable SWAP jobs (disabled by default)\n");
                printf("  -q : disable interactive display (enabled by default)\n");
                return EXIT_FAILURE;
        }
    }

    if (num_workers <= 0) {
        fprintf(stderr, "Error: Number of workers (-w) must be positive.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    pthread_mutex_init(&count_mutex, NULL);
    init_queue();
    init_flags();

    pthread_t display_tid;
    pthread_t* workers = malloc(num_workers * sizeof(pthread_t));

    if(!quiet)
        pthread_create(&display_tid, NULL, display_thread, NULL);

    for (int i = 0; i < num_workers; i++) {
        pthread_create(&workers[i], NULL, worker_thread, NULL);
    }

    // Master thread logic in main
    int num_types = enable_swap ? 3 : 2;
    for (int i = 0; i < num_jobs; i++) {
        struct Job job;
        job.id=i;
        job.type = rand() % num_types;

        if (job.type == JOB_SWAP) {
            job.idx1 = rand() % NUM_FLAGS;
            job.idx2 = rand() % NUM_FLAGS;
            while (job.idx1 == job.idx2) {
                job.idx2 = rand() % NUM_FLAGS;
            }
        } else {
            job.idx1 = rand() % NUM_FLAGS;
            job.idx2 = -1;
        }
        enqueue(job);
        if(delay) {
            usleep(delay);
        }
    }

    // Signal termination
    pthread_mutex_lock(&queue.mutex);
    queue.done = true;
    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);

    // Join workers
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    // Wait a moment and final display
    if(!quiet) {
        usleep(300000);
        pthread_cancel(display_tid);  // best effort stop
        pthread_join(display_tid, NULL);
    }

    // Final verification
    printf("\n\n=== Program finished ===\n");
    print_grid();

    // Cleanup
    free(workers);
    destroy_queue();
    destroy_mutexes();

    return EXIT_SUCCESS;
}