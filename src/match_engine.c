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
int generate_ball_event(Batsman* bat, Bowler* bowler)
{
    int skill = bat->job_length;
    int r = rand() % 100;

     // ───── NO BALL (2%) ─────
    if (r < 2) return 8;
    r -= 2;

    // ───── WIDE (6%) ─────
    if (r < 6) return 7;
    r -= 6;

    // ───── WICKET ─────
    int skill_diff = bat->job_length - bowler->skill;
    int wicket_prob = 8 - skill_diff / 6;

    if (wicket_prob < 2) wicket_prob = 2;
    if (wicket_prob > 15) wicket_prob = 15;

    if (r < wicket_prob) return -1;
    r -= wicket_prob;

    // ───── DOT ─────
    if (r < 18) return 0;
    r -= 18;

    // ───── SINGLES ─────
    if (r < 32) return 1;
    r -= 32;

    // ───── DOUBLES ─────
    if (r < 14) return 2;
    r -= 14;

    // ───── THREES ─────
    if (r < 3) return 3;
    r -= 3;

    // ───── BOUNDARIES ─────
    int four_prob = 16 + skill / 5;
    int six_prob  = 9 + skill / 8;

    if (r < four_prob) return 4;
    if (r < four_prob + six_prob) return 6;

    return 1;
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

        usleep(200);

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

        usleep(200);

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

    printf("        [KERNEL] Deadlock detected → resolving via run-out\n");

    pthread_mutex_unlock(&print_mutex);

    /* reset flags */
    pthread_mutex_lock(&deadlock_mutex);
    striker_waiting    = 0;
    nonstriker_waiting = 0;
    pthread_mutex_unlock(&deadlock_mutex);

    /* Force-release end1 and end2 only if they are currently locked.
       attempt_run() releases the mutex it owns before returning 1
       (striker releases end1, non-striker releases end2), so by the
       time we get here both mutexes are ALREADY UNLOCKED.
       Calling pthread_mutex_unlock on an unlocked mutex you don't own
       is undefined behaviour and silently corrupts the mutex on Linux,
       causing future lock calls to hang permanently.
       Use trylock: if it succeeds we own it and can safely unlock it;
       if it fails someone else holds it and we leave it alone. */
    if (pthread_mutex_trylock(&end1_mutex) == 0)
        pthread_mutex_unlock(&end1_mutex);

    if (pthread_mutex_trylock(&end2_mutex) == 0)
        pthread_mutex_unlock(&end2_mutex);
}