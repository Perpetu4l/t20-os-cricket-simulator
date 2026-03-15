#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int generate_ball_event() {

    int r = rand() % 100;

    if(r < 20) return -1;       // wicket
    if(r < 30) return 0;       // dot ball
    if(r < 60) return 1;       // single
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
int attempt_run(int runs)
{
    for(int i = 0; i < runs; i++)
    {
        /* striker leaves End1 */
        pthread_mutex_lock(&end1_mutex);

        /* simulate non-striker already occupying End2 */
        int simulate_conflict = rand() % 4;   // 25% chance

        if(simulate_conflict == 0)
        {
            pthread_mutex_unlock(&end1_mutex);
            return 1;   // run-out due to circular wait
        }

        /* otherwise striker successfully reaches End2 */
        pthread_mutex_lock(&end2_mutex);

        sleep(1);  // running delay

        pthread_mutex_unlock(&end2_mutex);
        pthread_mutex_unlock(&end1_mutex);
    }

    return 0;
}