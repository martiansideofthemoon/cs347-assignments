
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10
#define TOTAL 1000

int count[20];
int total_count = 0;
int files_left = NUM_THREADS;
int COUNT_LIMIT = 50;

pthread_mutex_t count_mutex, files_mutex;
pthread_cond_t count_threshold_cv, count_wait;

void *inc_count(void *_file) {
    char filename[80];
    sscanf((char*)_file, "%s", filename);
    FILE *file = fopen(filename,"r");
    if(!file) {
        printf("Error opening file %s!\n", filename);
        pthread_exit(NULL);
    }
    int party, local_count;

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

        // we're using a common mutex for all parties to ensure that `count[]` updates
        // synchronously. This will ensure `total_count` outputs at the correct timesteps
        pthread_mutex_lock(&count_mutex);
        // The `while` loop is essential since a number of threads are waiting on this condition
        // Votes will be counted ONLY when its sure that total_count < COUNT_LIMIT
        while(total_count >= COUNT_LIMIT) {
            pthread_cond_wait(&count_wait, &count_mutex);
        }
        count[party - 1] += local_count;
        total_count += local_count;
        if(total_count >= COUNT_LIMIT) {
            // Carry out printing process in `wait_count` thread. As long as this condition
            // remains true, counter threads are blocked until a print takes place
            pthread_cond_signal(&count_threshold_cv);
        }
        pthread_mutex_unlock(&count_mutex);
    }
    fclose(file);
    pthread_mutex_lock(&files_mutex);
    files_left -= 1;
    if (files_left == 0) {
        // Precaution to prevent infinite wait
        pthread_cond_signal(&count_threshold_cv);
    }
    pthread_mutex_unlock(&files_mutex);
    pthread_exit(NULL);
}


void* wait_count(void* t) {
    pthread_mutex_lock(&count_mutex);
    while(files_left > 0) {
        // Thread blocked till enough votes counted for printing
        pthread_cond_wait(&count_threshold_cv, &count_mutex);
        if (total_count < COUNT_LIMIT) {
            continue;
        }
        int sum = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                sum += count[5 * i + j];
                printf("%i ", count[5 * i + j]);
            }
            printf("\b\t");
        }
        // Uncomment this to prove code's correctness!
        //printf(" - %i\n", sum);
        printf("\n");

        COUNT_LIMIT += 50;
        // COUNT_LIMIT has been raised and results have been printed
        // vote counting can continue
        pthread_cond_broadcast(&count_wait);
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

    if(!filename_list) {
        perror("No such file\n");
        exit(1);
    }

    int iter_f = 0;
    while(fgets(filename[iter_f], sizeof(filename[iter_f]), filename_list) && iter_f < 10) {
        iter_f++;
    }

    pthread_t wait_thread;
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    // Initialize mutex and conditional variable
    pthread_mutex_init(&count_mutex, NULL);
    pthread_mutex_init(&files_mutex, NULL);
    pthread_cond_init(&count_wait, NULL);
    pthread_cond_init(&count_wait, NULL);

    // joined state
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&wait_thread, &attr, wait_count, NULL);
    for(int iter = 0;iter < NUM_THREADS; ++iter) {
        pthread_create(&threads[iter], &attr, inc_count, (void *)filename[iter]);
    }

    // Wait for all threads to complete
    for(int i = 0;i<NUM_THREADS;++i) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(wait_thread, NULL);

    // Printing final values
    for(int i = 0;i<4;i++) {
        for(int j = 0;j<5;j++)
            printf("%i ", count[5*i+j]);
        printf("\b\t");
    }
    printf("\n");

    // Clean up
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&files_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_cond_destroy(&count_wait);
    pthread_exit(NULL);

    return 0;
}