#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#define NUM_BINS 5
#define MAX_QUEUE 10000
#define USLEEP_USEC 1000  // 10 ms delay for UAF reproducibility (buggy version only)

typedef struct {
    int id;
    int type;  // 0: simple, 1: count-increment, 2: milestone-alert, 3: inventory-transfer
    int target_seq;      // for milestone-alert
    int src_bin;         // for transfer
    int dst_bin;         // for transfer
} Order;

Order *queue[MAX_QUEUE];
int queue_size = 0;
int queue_front = 0;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

int global_sequence = 0;              // BUG 2: classic race on shared integer count (unprotected increments)
int wakeup_seq = -1;
bool inventory[NUM_BINS] = {false};   // each bin holds 0 or 1 item
pthread_mutex_t bin_locks[NUM_BINS];
pthread_mutex_t inventory_lock = PTHREAD_MUTEX_INITIALIZER;  // only for final/controller checks

pthread_cond_t alert_cond = PTHREAD_COND_INITIALIZER;  // single CV for all milestone-alert tasks
pthread_mutex_t alert_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t round_barrier;      // BUG 7: barrier initialized to wrong count

int num_workers_global;
int expected_total = 0;
int task_group_global = 1;
int tasks_remaining = 0;
pthread_cond_t completion_cond = PTHREAD_COND_INITIALIZER;

void *worker(void *arg);
void enqueue_orders(int num_tasks, int task_group);
void controller_checks();

int main(int argc, char **argv) {
    int num_workers = 1;   // default
    int num_tasks = 0;
    int run_seconds = 0;
    int task_group = 1;

    int opt;
    while ((opt = getopt(argc, argv, "w:n:s:g:")) != -1) {
        switch (opt) {
            case 'w': num_workers = atoi(optarg); break;
            case 'n': num_tasks = atoi(optarg); break;
            case 's': run_seconds = atoi(optarg); break;
            case 'g': task_group = atoi(optarg); break;
            default:
                fprintf(stderr, "Usage: %s -w <num_workers 1-4> -n <num_tasks> -s <run_seconds> -g <task_group>\n", argv[0]);
                return 1;
        }
    }

    if (num_workers < 1 || num_workers > 4) {
        fprintf(stderr, "num_workers must be 1-4\n");
        return 1;
    }

    num_workers_global = num_workers;
    task_group_global = task_group;

    for (int i = 0; i < NUM_BINS; i++) {
        pthread_mutex_init(&bin_locks[i], NULL);
        inventory[i] = (i == 0);  // initial: one item in bin 0
    }

    // BUG 7: barrier initialized to wrong count (should be num_workers)
    pthread_barrier_init(&round_barrier, NULL, num_workers + 1);  // incorrect

    pthread_t workers[4];
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&workers[i], NULL, worker, NULL);
    }

    enqueue_orders(num_tasks, task_group_global);

    if (run_seconds > 0) {
        sleep(run_seconds);
    } else {
        pthread_mutex_lock(&queue_lock);
        while (tasks_remaining > 0) {
            pthread_cond_wait(&completion_cond, &queue_lock);
        }
        printf("remaining: %d\n",tasks_remaining);
        pthread_mutex_unlock(&queue_lock);
    }

    pthread_cond_broadcast(&queue_cond);

    // BUG 6: use-after-free - immediately free remaining queue while workers may still hold pointers
    // for (int i = queue_front; i < queue_size; i++) {
    //     free(queue[i]);
    // }
    queue_size = 0;

    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    controller_checks();  // final check
    printf("FINAL CHECKSUM: %d\n", global_sequence);  // for lab verification

    // cleanup
    for (int i = 0; i < NUM_BINS; i++) pthread_mutex_destroy(&bin_locks[i]);
    return 0;
}

void *worker(void *arg) {
    (void)arg;    
    while (1) {

        pthread_mutex_lock(&queue_lock);
        while (queue_front == queue_size) {
            pthread_cond_wait(&queue_cond, &queue_lock);  
        }                        
        if(queue_front > queue_size) break;        
        Order *o = queue[queue_front++];
        pthread_mutex_unlock(&queue_lock);

        // Process according to type
        if (o->type == 1) {
            // BUG 2: classic race on shared count
            global_sequence++;
            if(global_sequence == wakeup_seq)
                pthread_cond_broadcast(&alert_cond);
        } else if (o->type == 2) {
            printf("Waiting for %d\n",o->target_seq);
            pthread_mutex_lock(&alert_lock);
            wakeup_seq = o->target_seq;
            pthread_cond_wait(&alert_cond, &alert_lock);  
            printf("ALERT: milestone %d reached (actual observed = %d)\n", o->target_seq, global_sequence);
            pthread_mutex_unlock(&alert_lock);
        } else if (o->type == 3) {
            // BUG 5: deadlock (parameter-dependent lock ordering via bin indices)
            int first = o->src_bin;
            int second = o->dst_bin;                
            pthread_mutex_lock(&bin_locks[first]);
            pthread_mutex_lock(&bin_locks[second]);  // can deadlock if crossed with another thread

            // BUG 5 continued: critical section requires BOTH locks for atomic check + update
            bool can_transfer = inventory[o->src_bin] && !inventory[o->dst_bin];
            if (can_transfer) {
                inventory[o->src_bin] = false;
                inventory[o->dst_bin] = true;
            } else {
                // safe abort
            }

            pthread_mutex_unlock(&bin_locks[second]);
            pthread_mutex_unlock(&bin_locks[first]);

            // BUG 6 continued: use-after-free possible here (o is already freed by controller in timed mode)
            // (accessing o->id below would read freelist pointers)
            if (o->id % 100 == 0) printf("Processed transfer order %d\n", o->id);
        }

        free(o);  // normal free (the UAF happens on queue entries freed by controller)

        // signal completion for fixed-task mode
        pthread_mutex_lock(&queue_lock);
        tasks_remaining--;
        if (tasks_remaining == 0) {
            pthread_cond_signal(&completion_cond);
            pthread_mutex_unlock(&queue_lock);
            break;
        }
        pthread_mutex_unlock(&queue_lock);
    }

    return NULL;
}

// THERE ARE NO BUGS BELOW THIS LINE
// It may look a little funny, but that's just because it's setting up a scenario that triggers
// concurrency bugs elsewhere, not because there are bugs here. Don't change code below. 
// THERE ARE NO BUGS BELOW THIS LINE

void controller_checks() {
    pthread_mutex_lock(&inventory_lock);
    int total_items = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        total_items += inventory[i] ? 1 : 0;
    }
    pthread_mutex_unlock(&inventory_lock);

    if (global_sequence != expected_total) {  // expected deterministic value for demo
        fprintf(stderr, "INVARIANT VIOLATION: sequence = %d (expected %d)\n",
                global_sequence, expected_total);
        exit(1);
    }
    if (total_items != 1) {  // invariant: exactly one item total
        fprintf(stderr, "INVARIANT VIOLATION: inventory total = %d (expected 1)\n",
                 total_items);
        exit(1);
    }
    printf("sequence = %d, inventory total = %d\n", global_sequence, total_items);
}

void enqueue_orders(int num_tasks, int task_group) {
    pthread_mutex_lock(&queue_lock);
    int to_enqueue = (num_tasks > 0 ? num_tasks : MAX_QUEUE);
    int i = 0;
    for (; i < to_enqueue; i++) {
        Order *o = malloc(sizeof(Order));
        o->id = i;

        // pure sequence number update
        if(task_group == 1) {
            o->type = 1;
            expected_total++;
        }
        // milestone alerts and sequence numbers
        else if(task_group == 2) {
            if(i%5100==0) {
                o->type = 2; // alert
                o->target_seq = 5000+i/100;
            }
            else {
                o->type = 1;
                expected_total++;
            }
        }        
        else if(task_group == 3){
            o->type = 3;
            o->src_bin = i % NUM_BINS;
            o->dst_bin = (i * 3) % NUM_BINS;
            if(o->src_bin==o->dst_bin) {
                o->dst_bin = (o->dst_bin+1)%NUM_BINS;
            }
        }
        queue[queue_size++] = o;
    }
    tasks_remaining = queue_size;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
}
