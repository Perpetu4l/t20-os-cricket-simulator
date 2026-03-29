#include "../include/simulator.h"

pthread_mutex_t pitch_mutex;
pthread_mutex_t score_mutex;
pthread_mutex_t fielder_mutex;
pthread_mutex_t print_mutex;
pthread_cond_t ball_hit_cond;
pthread_mutex_t crease_mutex;
pthread_mutex_t end1_mutex;
pthread_mutex_t end2_mutex;
pthread_mutex_t deadlock_mutex;

int striker_waiting = 0;
int nonstriker_waiting = 0;
int run_ready = 0;
pthread_mutex_t run_mutex;
pthread_cond_t run_cond;
pthread_mutex_t crease_state_mutex;
pthread_mutex_t batsman_mutex;
pthread_cond_t batsman_cond;

sem_t crease_sem;

void init_sync() {

    pthread_mutex_init(&pitch_mutex,NULL);
    pthread_mutex_init(&score_mutex,NULL);
    pthread_mutex_init(&fielder_mutex,NULL);
pthread_mutex_init(&crease_state_mutex, NULL);
    pthread_cond_init(&ball_hit_cond,NULL);
    pthread_mutex_init(&print_mutex, NULL);
    pthread_mutex_init(&end1_mutex,NULL);
    pthread_mutex_init(&end2_mutex,NULL);

pthread_mutex_init(&batsman_mutex, NULL);
pthread_cond_init(&batsman_cond, NULL);
    pthread_mutex_init(&deadlock_mutex, NULL);

    pthread_mutex_init(&run_mutex, NULL);
pthread_cond_init(&run_cond, NULL);

pthread_mutex_init(&crease_mutex, NULL);



    sem_init(&crease_sem,0,2);
}