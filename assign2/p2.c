#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

int count[20];
int total_count = 0;
int COUNT_LIMIT = 50;
int thread_count = 0;

pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void *inc_count(void *_file) {

    char filename[80];
    sscanf((char*)_file, "%s", filename);
    FILE *file = fopen(filename,"r");
    if(!file) {
        printf("Error opening file!\n");
        pthread_exit(NULL);
    }
    int party, local_count;

    pthread_mutex_lock(&count_mutex);
    //printf("This is thread modifying count from %i\n", total_count);
    char sentence[80];
    fgets(sentence, sizeof(sentence), file);
    while(fgets(sentence, sizeof(sentence), file)) {
        if(sentence[0]=='\n') {
            // get the district number and let it go
            fgets(sentence, sizeof(sentence), file);
            continue;
        }
        sscanf(sentence, "%i%i", &party, &local_count);
        count[party-1] += local_count;
        total_count += local_count;
        if(total_count>=COUNT_LIMIT) {
            pthread_cond_signal(&count_threshold_cv);
        }
    }

    fclose(file);
    thread_count++;
    
    // if last incremental thread
    // make sure watch_count is aware
    if(thread_count == 10)
        pthread_cond_signal(&count_threshold_cv);

    pthread_mutex_unlock(&count_mutex);
    sleep(1);

    pthread_exit(NULL);
}

void *watch_count(void *dummy) {

    pthread_mutex_lock(&count_mutex);
    while(thread_count<10) {
        pthread_cond_wait(&count_threshold_cv, &count_mutex);

        for(int i = 0;i<4;i++) {
            for(int j = 0;j<5;j++)
                printf("%i ", count[5*i+j]);
            printf("\b\t");
        }
        printf("\n");
        COUNT_LIMIT = 50*((total_count%50)+1);
    }
    pthread_mutex_unlock(&count_mutex);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    // Initialize counts
    for( int i = 0;i<20;++i ) {
        count[i] = 0;
    }

    char filename[NUM_THREADS][80];

    FILE *filename_list;
    filename_list = fopen("filename_list.txt", "r");

    
    int iter_f = 0;
    while(fgets(filename[iter_f], sizeof(filename[iter_f]), filename_list) && iter_f < 10) {
        iter_f++;
    }

    pthread_t threads[NUM_THREADS+1];
    pthread_attr_t attr;

    // Initialize mutex and conditional variable
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);

    // joined state
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int dummy;
    pthread_create(&threads[0], &attr, watch_count, (void *)dummy);

    for(int iter = 0;iter < NUM_THREADS; ++iter) {
        pthread_create(&threads[iter+1], &attr, inc_count, (void *)filename[iter]);
    }

    // Wait for all threads to complete
    for(int i = 0;i<NUM_THREADS;++i) {
        pthread_join(threads[i], NULL);
    }
    // printf("Final value of count is %i\n", count);

    // Clean up
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit(NULL);

    return 0;
}
