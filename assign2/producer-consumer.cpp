#include <stdlib.h>        // for srand, rand
#include <unistd.h>
#include "time.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <list>
#include <string>
#include <vector>
#define PRODUCERS 3
#define CONSUMERS 2

using namespace std;

bool all_true(bool* a, int size) {
    bool all = true;
    for (int i = 0; i < size; i++) {
        if (!a[i]) {
            all = false;
            break;
        }
    }
    return all;
}

class Producer {
public:
    int id;
    list<int> overflow;
    list<int> origin;
    int oldest;

    Producer() {
        oldest = 0;
    }

    Producer(int i) {
        id = i;
        oldest = 0;
    }

    void push_item(int id, int cycle) {
        overflow.push_back(id);
        origin.push_back(cycle);
    }

    int get_item() {
        int item = overflow.front();
        overflow.pop_front();
        origin.pop_front();
        if (origin.size() > 0) {
            oldest = origin.front();
        } else {
            oldest++;
        }
        return item;
    }

};

class Consumer {
public:
    int pending;

    Consumer() {
        pending = 0;
    }

    void add_jobs(int items) {
        pending += items;
    }

    void remove_job() {
        pending--;
    }

};

int buffer_cap;
list<int> buffer;
int new_orders[PRODUCERS + CONSUMERS];
int item_count = 0;
int cycle = 0;
bool input_being_read = true;
bool input_left = true;
bool deadlock = false;
bool preprocessing_done[PRODUCERS + CONSUMERS] = {false, false, false, false, false};
bool can_execute[PRODUCERS + CONSUMERS] = {false, false, false, false, false};

Producer producers[PRODUCERS];
Consumer consumers[CONSUMERS];

pthread_mutex_t item_mutex, input_mutex, preprocess_mutex, deadlock_mutex;
pthread_cond_t can_execute_cv[PRODUCERS + CONSUMERS];
pthread_cond_t input_cv, preprocess_cv, deadlock_cv;

int produce_item() {
    item_count++;
    return item_count;
}

void producer(int id) {
    while(input_left) {
        // Waiting for next line of input to be processed
        pthread_mutex_lock(&input_mutex);
        while(input_being_read == true) {
            pthread_cond_wait(&input_cv, &input_mutex);
        }
        pthread_mutex_unlock(&input_mutex);
        if (input_left == false) {
            break;
        }

        // Producing all items
        for (int i = 0; i < new_orders[id]; i++) {
            pthread_mutex_lock(&item_mutex);
                int item = produce_item();
            pthread_mutex_unlock(&item_mutex);
            producers[id].push_item(item, cycle);
        }

        // Signalling moderator thread that preprocessing is done
        pthread_mutex_lock(&preprocess_mutex);
            preprocessing_done[id] = true;
            pthread_cond_signal(&preprocess_cv);
        pthread_mutex_unlock(&preprocess_mutex);

        // Keep doing this until len(buffer) = buffer_cap and sum(consumer.pending) == 0
        // or sum(len(producer.overflow)) = 0
        // Make sure at every stage, min(producer.oldest) is placed in queue
        pthread_mutex_lock(&deadlock_mutex);
        while(true) {
            while(can_execute[id] == false)
                pthread_cond_wait(&can_execute_cv[id], &deadlock_mutex);
            can_execute[id] = false;
            if (deadlock) {
                break;
            }
            // Check whether producer contains oldest item
            int oldest = -1;
            for (int i = 0; i < PRODUCERS; i++) {
                if (oldest == -1 || producers[i].oldest < oldest) {
                    oldest = producers[i].oldest;
                }
            }
            if (producers[id].overflow.size() == 0) continue;
            if (producers[id].oldest != oldest) continue;
            if (buffer.size() == buffer_cap) continue;

            // This action can take place, proceed!
            int item = producers[id].get_item();
            buffer.push_back(item);
            for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
                can_execute[i] = false;
            }
            pthread_cond_signal(&deadlock_cv);
        }
        pthread_mutex_unlock(&deadlock_mutex);
    }
}

void consumer(int id) {
    while(input_left) {
        // Waiting for next line of input to be processed
        pthread_mutex_lock(&input_mutex);
        while(input_being_read == true) {
            pthread_cond_wait(&input_cv, &input_mutex);
        }
        pthread_mutex_unlock(&input_mutex);
        if (input_left == false) {
            break;
        }

        // Adding requested jobs to `pending`
        consumers[id - PRODUCERS].add_jobs(new_orders[id]);

        // Signalling moderator thread that preprocessing is done
        pthread_mutex_lock(&preprocess_mutex);
            preprocessing_done[id] = true;
            pthread_cond_signal(&preprocess_cv);
        pthread_mutex_unlock(&preprocess_mutex);

        // Keep doing this until `pending` is 0, or len(buffer) + sum(len(producer.overflow)) = 0
        pthread_mutex_lock(&deadlock_mutex);
        while(true) {
            while(can_execute[id] == false)
                pthread_cond_wait(&can_execute_cv[id], &deadlock_mutex);
            can_execute[id] = false;
            if (deadlock) {
                break;
            }
            // if continue executes, this thread will wait for its next turn
            if (consumers[id - PRODUCERS].pending == 0) continue;
            if (buffer.size() == 0) continue;
            // This action can take place, proceed!
            buffer.pop_front();
            consumers[id - PRODUCERS].remove_job();
            for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
                can_execute[i] = false;
            }
            pthread_cond_signal(&deadlock_cv);
        }
        pthread_mutex_unlock(&deadlock_mutex);
    }
}

void moderator() {
    string line;
    while (getline(cin, line)) {
        pthread_mutex_lock(&input_mutex);
        istringstream iss(line);
        for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
            iss >> new_orders[i];
        }
        for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
            preprocessing_done[i] = false;
        }
        input_being_read = false;
        pthread_cond_broadcast(&input_cv);
        pthread_mutex_unlock(&input_mutex);

        // Waiting for producer and consumer thread to carry out preprocessing
        pthread_mutex_lock(&preprocess_mutex);
            while(!all_true(preprocessing_done, PRODUCERS + CONSUMERS)) {
                pthread_cond_wait(&preprocess_cv, &preprocess_mutex);
            }
        pthread_mutex_unlock(&preprocess_mutex);
        deadlock = false;
        // Now that preprocessing is done, we can begin placing items into buffer
        // At each stage, check whether system is deadlocked. If not, allow threads to execute
        pthread_mutex_lock(&deadlock_mutex);

        // This is done to prevent the threads from running into next cycle of execution
        input_being_read = true;

        while(deadlock == false) {
            int produce_left = buffer.size();
            int overflow_size = 0;
            for (int i = 0; i < PRODUCERS; i++) {
                overflow_size += producers[i].overflow.size();
            }
            produce_left += overflow_size;
            int consume_left = 0;
            for (int i = 0; i < CONSUMERS; i++) {
                consume_left += consumers[i].pending;
            }
            if (consume_left > 0 && produce_left > 0) {
                // Deadlock situation not yet reached, `buffer` transactions needed
                for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
                    can_execute[i] = true;
                    pthread_cond_signal(&can_execute_cv[i]);
                }
                pthread_cond_wait(&deadlock_cv, &deadlock_mutex);
            } else if (buffer.size() < buffer_cap && overflow_size > 0) {
                // Deadlock situation not yet reached, `buffer` transactions needed
                for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
                    can_execute[i] = true;
                    pthread_cond_signal(&can_execute_cv[i]);
                }
                pthread_cond_wait(&deadlock_cv, &deadlock_mutex);
            }
            else {
                deadlock = true;
                for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
                    can_execute[i] = true;
                    pthread_cond_signal(&can_execute_cv[i]);
                }
            }
        }
        pthread_mutex_unlock(&deadlock_mutex);

        // Now that cycle is complete, print overflow and buffer items
        cout << "Buffer Items: ";
        list<int>::iterator it = buffer.begin();
        while(it != buffer.end()) {
            cout << *it << " ";
            it++;
        }
        cout << endl << "Overflow Items: ";
        for (int i = 0; i < PRODUCERS; i++) {
            it = producers[i].overflow.begin();
            while(it != producers[i].overflow.end()) {
                cout << *it << " ";
                it++;
            }
        }
        cout << endl << endl;
        cycle++;
    }
    input_left = false;
    input_being_read = false;
    // Safety check to ensure all threads finish successfully
    pthread_cond_broadcast(&input_cv);
}

void *thread_run(void *data){
    int thread_id = *((int *)data);
    if (thread_id < PRODUCERS) {
        producer(thread_id);
    } else if (thread_id >= PRODUCERS && thread_id < (PRODUCERS + CONSUMERS)) {
        consumer(thread_id);
    } else {
        moderator();
    }
}

int main(int argc, char *argv[]) {
    pthread_mutex_init(&item_mutex, NULL);
    pthread_mutex_init(&input_mutex, NULL);
    pthread_mutex_init(&deadlock_mutex, NULL);
    pthread_cond_init(&input_cv, NULL);
    pthread_cond_init(&preprocess_cv, NULL);
    pthread_cond_init(&deadlock_cv, NULL);
    for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
        pthread_cond_init(&can_execute_cv[i], NULL);
    }
    // Taking care of buffer cap value
    string line;
    getline(cin, line);
    istringstream iss(line);
    iss >> buffer_cap;

    int total_threads = PRODUCERS + CONSUMERS + 1;

    vector<pthread_t> thr(total_threads);

    for (int i = 0; i < PRODUCERS; i++) {
        producers[i] = *(new Producer(i));
    }
    for (int i = 0; i < CONSUMERS; i++) {
        consumers[i] = *(new Consumer());
    }

    vector<int> tid(total_threads);
    for(int i = 0; i < total_threads; i++){
        tid[i] = i;
        pthread_create(&thr[i], NULL, thread_run, (void *)&tid[i]);
    }

    for(int i=0; i < total_threads; i++){
        pthread_join(thr[i], NULL);
    }

    pthread_mutex_destroy(&item_mutex);
    pthread_mutex_destroy(&input_mutex);
    pthread_mutex_init(&deadlock_mutex, NULL);
    pthread_cond_destroy(&input_cv);
    pthread_cond_destroy(&preprocess_cv);
    pthread_cond_destroy(&deadlock_cv);
    for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
        pthread_cond_destroy(&can_execute_cv[i]);
    }
    exit(0);
}