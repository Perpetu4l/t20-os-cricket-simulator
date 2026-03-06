#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int generate_ball_event() {

    int r = rand() % 100;

    if(r < 5) return -1;       // wicket
    if(r < 25) return 0;       // dot ball
    if(r < 50) return 1;       // single
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

        int next = sjf_scheduler();

        pthread_mutex_lock(&print_mutex);
        printf("SJF Scheduler selected batsman %d\n", next);
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
