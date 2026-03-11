#include "../include/simulator.h"

pthread_mutex_t pitch_mutex;
pthread_mutex_t score_mutex;
pthread_mutex_t fielder_mutex;
pthread_mutex_t print_mutex;
pthread_cond_t ball_hit_cond;

sem_t crease_sem;

void init_sync() {

    pthread_mutex_init(&pitch_mutex,NULL);
    pthread_mutex_init(&score_mutex,NULL);
    pthread_mutex_init(&fielder_mutex,NULL);

    pthread_cond_init(&ball_hit_cond,NULL);
    pthread_mutex_init(&print_mutex, NULL);
    

    sem_init(&crease_sem,0,2);
}