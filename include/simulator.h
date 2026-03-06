#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_FIELDERS 10
#define MAX_BATSMEN 11
#define MAX_BOWLERS 5
#define MAX_OVERS 20
#define DEATH_OVER_BOWLER 4

typedef struct {
    int runs;
    int wickets;
    int overs;
    int balls;
} Scoreboard;

typedef struct {
    int id;
    int runs;
    int balls_faced;
    int is_out;
    int job_length;
} Batsman;

typedef struct {
    int id;
    int balls_bowled;
    int runs_given;
    int wickets;
} Bowler;

typedef struct {
    Scoreboard score;
    int match_running;
    int ball_in_air;
} MatchState;

extern MatchState match;
int generate_ball_event();
void update_score(int result);
void log_ball(int over,int ball,int result);
void init_sync();


/* pitch buffer */
extern int pitch_ball;

/* synchronization */
extern pthread_mutex_t pitch_mutex;

extern pthread_mutex_t score_mutex;

extern pthread_mutex_t print_mutex;


extern sem_t crease_sem;

extern pthread_cond_t ball_hit_cond;
extern pthread_mutex_t fielder_mutex;


/* scheduler */

extern int current_bowler;
extern pthread_t bowler_threads[MAX_BOWLERS];
extern Batsman batsmen[MAX_BATSMEN];

void round_robin_scheduler();
int sjf_scheduler();
void priority_scheduler();


#endif