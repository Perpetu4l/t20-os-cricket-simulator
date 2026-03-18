
#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int generate_ball_event() {

    int r = rand() % 100;

    if(r < 20) return -1;       // wicket
    if(r < 30) return 0;       // dot ball
    if(r < 90) return 1;       // single
    if(r < 65) return 2;       // double
    if(r < 85) return 4;       // four
    if(r < 95) return 6;       // six
    return 7;                  // wide
}

void update_score(int result) {

    pthread_mutex_lock(&score_mutex);



    if(result == -1) {

        match.score.wickets++;

        pthread_mutex_lock(&print_mutex);
        printf("WICKET!\n");
        pthread_mutex_unlock(&print_mutex);
    }   

    else if(result == 7) {

        match.score.runs += 1;

        pthread_mutex_lock(&print_mutex);
        printf("WIDE BALL\n");
        pthread_mutex_unlock(&print_mutex);
    }
    else {

        match.score.runs += result;
    }

    pthread_mutex_unlock(&score_mutex);
}

void swap_strike(){

    int temp = match.striker;
    match.striker = match.non_striker;
    match.non_striker = temp;
}

int attempt_run(int thread_id)
{
    if(thread_id == 0)   /* striker */
    {
        pthread_mutex_lock(&end1_mutex);

        usleep(10000);

        if(pthread_mutex_trylock(&end2_mutex) != 0)
        {
            pthread_mutex_lock(&deadlock_mutex);
            striker_waiting = 1;
            pthread_mutex_unlock(&deadlock_mutex);
            return 1;
        }

        pthread_mutex_unlock(&end2_mutex);
        pthread_mutex_unlock(&end1_mutex);
    }
    else   /* non-striker */
    {
        pthread_mutex_lock(&end2_mutex);

        usleep(10000);

        if(pthread_mutex_trylock(&end1_mutex) != 0)
        {
            pthread_mutex_lock(&deadlock_mutex);
            nonstriker_waiting = 1;
            pthread_mutex_unlock(&deadlock_mutex);
            return 1;
        }

        pthread_mutex_unlock(&end1_mutex);
        pthread_mutex_unlock(&end2_mutex);
    }

    return 0;
}

int detect_deadlock()
{
    pthread_mutex_lock(&deadlock_mutex);

    int deadlock = striker_waiting && nonstriker_waiting;

    pthread_mutex_unlock(&deadlock_mutex);

    return deadlock;
}
void resolve_deadlock()
{
    int victim;

    pthread_mutex_lock(&deadlock_mutex);

    if(rand() % 2)
        victim = match.striker;
    else
        victim = match.non_striker;

    pthread_mutex_unlock(&deadlock_mutex);

    printf("UMPIRE DETECTED DEADLOCK\n");
    printf("RUN OUT! Batsman %d is run out due to deadlock\n", victim);

    batsmen[victim].is_out = 1;
    update_score(-1);

    /* force release of crease resources */
    pthread_mutex_unlock(&end1_mutex);
    pthread_mutex_unlock(&end2_mutex);

    pthread_mutex_lock(&deadlock_mutex);
    striker_waiting = 0;
    nonstriker_waiting = 0;
    pthread_mutex_unlock(&deadlock_mutex);

    int next = sjf_scheduler();

    printf("SJF Scheduler selected batsman %d\n", next);

    if(victim == match.striker)
        match.striker = next;
    else
        match.non_striker = next;
}