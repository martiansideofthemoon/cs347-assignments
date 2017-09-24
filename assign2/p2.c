#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 3
#define COUNT_LIMIT 12

int count = 0;

pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;


void *inc_count(void *t) {
    int i;
    long my_id = (long) t;
    int personalLock = 1;

    while(personalLock){
        pthread_mutex_lock(&count_mutex);
        printf("This is thread %i modifying count from %i\n", my_id, count);
        count++;

        if(count >= COUNT_LIMIT) {
            pthread_cond_signal(&count_threshold_cv);
            personalLock = 0;
        }

        pthread_mutex_unlock(&count_mutex);

        sleep(1);
    }

    pthread_exit(NULL);
}

void *watch_count(void *t) {
    long my_id = (long) t;

    pthread_mutex_lock(&count_mutex);
    while(count < COUNT_LIMIT) {
        pthread_cond_wait(&count_threshold_cv, &count_mutex);
        printf("Signal received at count %i\n", count);
    }
    pthread_mutex_unlock(&count_mutex);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    long t1=1, t2=2, t3=3; // IDs?
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    // Initialize mutex and conditional variable
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);

    // joined state
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, watch_count, (void *)t1);
    pthread_create(&threads[1], &attr, inc_count, (void *)t2);
    pthread_create(&threads[2], &attr, inc_count, (void *)t3);

    // Wait for all threads to complete
    for(int i = 0;i<NUM_THREADS;++i) {
        pthread_join(threads[i], NULL);
    }
    printf("Final value of count is %i\n", count);

    // Clean up
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit(NULL);

    return 0;
}
