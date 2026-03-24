#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int toss_winner;     // 0 = team1, 1 = team2
int toss_decision;   // 0 = bat, 1 = bowl

void print_team(Team team){
    printf("\nTeam: %s\n", team.name);
    printf("--------------------------------\n");

    for(int i = 0; i < MAX_BATSMEN; i++){
        printf("%2d. %-15s (%s)\n",
            i+1,
            team.players[i].name,
            team.players[i].role
        );
    }

    printf("\nBowlers:\n");
    for(int i = 0; i < MAX_BOWLERS; i++){
        printf(" - %s\n", team.bowlers[i].name);
    }

    printf("\n");
}

void perform_toss(){

    toss_winner = rand() % 2;
    toss_decision = rand() % 2;

    printf("\n==============================\n");
    printf("         TOSS\n");
    printf("==============================\n");

    printf("%s won the toss\n",
        toss_winner == 0 ? team1.name : team2.name);

    if(toss_decision == 0){
        printf("Decision: BAT first\n");
    } else {
        printf("Decision: BOWL first\n");
    }

    printf("\n");
}


int generate_ball_event(Batsman* bat){

    int skill = bat->job_length;  // 50 → top order, 3 → tail
    int r = rand() % 100;

    int wicket_prob = 40 - (skill / 2);  // better batsman = safer

    if(r < wicket_prob) return -1;
    if(r < wicket_prob + 20) return 0;
    if(r < wicket_prob + 40) return 1;
    if(r < wicket_prob + 55) return 2;
    if(r < wicket_prob + 75) return 4;
    if(r < wicket_prob + 90) return 6;

    return 7;
}


void update_score(int result) {

    pthread_mutex_lock(&score_mutex);

    if(result == -1) {
        match.score.wickets++;
    }   
    else if(result == 7) {
        match.score.runs += 1;
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

/* ================= RUN LOGIC ================= */

int attempt_run(int thread_id)
{
    if(thread_id == 0)
    {
        pthread_mutex_lock(&end1_mutex);

        usleep(10000);

        if(pthread_mutex_trylock(&end2_mutex) != 0)
        {
            pthread_mutex_lock(&deadlock_mutex);
            striker_waiting = 1;
            pthread_mutex_unlock(&deadlock_mutex);

            pthread_mutex_unlock(&end1_mutex);
            return 1;
        }

        pthread_mutex_unlock(&end2_mutex);
        pthread_mutex_unlock(&end1_mutex);

        pthread_mutex_lock(&deadlock_mutex);
        striker_waiting = 0;
        pthread_mutex_unlock(&deadlock_mutex);
    }
    else
    {
        pthread_mutex_lock(&end2_mutex);

        usleep(10000);

        if(pthread_mutex_trylock(&end1_mutex) != 0)
        {
            pthread_mutex_lock(&deadlock_mutex);
            nonstriker_waiting = 1;
            pthread_mutex_unlock(&deadlock_mutex);

            pthread_mutex_unlock(&end2_mutex);
            return 1;
        }

        pthread_mutex_unlock(&end1_mutex);
        pthread_mutex_unlock(&end2_mutex);

        pthread_mutex_lock(&deadlock_mutex);
        nonstriker_waiting = 0;
        pthread_mutex_unlock(&deadlock_mutex);
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

/* ================= DEADLOCK HANDLER ================= */

void resolve_deadlock()
{
    pthread_mutex_lock(&print_mutex);

    printf("\n  [UMPIRE/KERNEL] *** DEADLOCK DETECTED: RUN-OUT ***\n\n");

    pthread_mutex_unlock(&print_mutex);

    /* reset flags */
    pthread_mutex_lock(&deadlock_mutex);
    striker_waiting = 0;
    nonstriker_waiting = 0;
    pthread_mutex_unlock(&deadlock_mutex);

    /* force release locks (safe for recovery simulation) */
    pthread_mutex_unlock(&end1_mutex);
    pthread_mutex_unlock(&end2_mutex);
}