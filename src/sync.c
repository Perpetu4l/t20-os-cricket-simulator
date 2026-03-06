#include "../include/simulator.h"

pthread_mutex_t pitch_mutex;

void init_sync() {
    pthread_mutex_init(&pitch_mutex,NULL);
}