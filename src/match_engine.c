#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int toss_winner;    
int toss_decision; 

void print_team(Team team){
    printf("\nTeam: %s\n", team.name);
    printf("--------------------------------\n");

    for(int i = 0; i < 11; i++){
        printf("%2d. %-15s (%s)\n",
            i+1,
            team.players[i].name,
            team.players[i].role
        );
    }

    printf("\nBowlers:\n");
    for(int i = 0; i < 6; i++){
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

    printf("%s won the toss\n",toss_winner == 0 ? team1.name : team2.name);

    if(toss_decision == 0){
        printf("Decision: BAT first\n");
    } else {
        printf("Decision: BOWL first\n");
    }

    printf("\n");
}

int generate_ball_event(Batsman* bat, Bowler* bowler)
{
    int skill = bat->job_length;
    int r = rand() % 100;
    if (r < 1) return 8; 
    r -= 1;

    if (r < 3) return 7; 
    r -= 3;
    int skill_diff = skill - bowler->skill;

    int pw = 5 - skill_diff / 8;

    if (pw < 2) pw = 2;
    if (pw > 10) pw = 10;

    if (match.score.overs >= 19)pw += 2;

    if (r < pw) return -1;
    r -= pw;

    int p0 = 20 + (bowler->skill / 8);

    if (match.score.overs >= 19)p0 -= 3; 

    if (p0 > 40) p0 = 40;
    if (p0 < 10) p0 = 10;

    if (r < p0) return 0;
    r -= p0;


    int p1 = 30 + (skill / 20);

    if (match.score.overs >= 19) p1 -= 5; 

    if (r < p1) return 1;
    r -= p1;


    int p2 = 10 + (skill / 25);
    if (r < p2) return 2;
    r -= p2;


    if (r < 2) return 3;
    r -= 2;

    int p4 = 8 + skill / 6;
    int p6  = 3 + skill / 10;

    if (match.score.overs >= 19) {
        p4 += 5;
        p6 += 4;
    }

    if (skill < 15) {
        p4 /= 2;
        p6 /= 3;
    }

    if (p4 > 30) p4 = 30;
    if (p6 > 15) p6 = 15;

    if (r < p4) return 4;
    if (r < p4 + p6) return 6;

    return 1;
}


void update_score(int result) {

    pthread_mutex_lock(&score_mutex);

    if(result == -1) { match.score.wickets++;
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



int attempt_run(int thread_id, int id)
{
    if(id == 0)
    {
        pthread_mutex_lock(&end1_mutex);

        usleep(300);

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

        usleep(300);

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


void resolve_deadlock()
{
    pthread_mutex_lock(&print_mutex);

    printf("        [KERNEL] Deadlock detected → resolving via run-out\n");

    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_lock(&deadlock_mutex);
    striker_waiting    = 0;
    nonstriker_waiting = 0;
    pthread_mutex_unlock(&deadlock_mutex);

    if (pthread_mutex_trylock(&end1_mutex) == 0) pthread_mutex_unlock(&end1_mutex);

    if (pthread_mutex_trylock(&end2_mutex) == 0) pthread_mutex_unlock(&end2_mutex);
}