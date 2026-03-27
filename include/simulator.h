#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define MAX_FIELDERS 10
#define MAX_BATSMEN 11
#define MAX_BOWLERS 5
#define MAX_OVERS 20
#define DEATH_OVER_BOWLER 4
#define MAX_BALL_EVENTS 800

typedef struct {
    int over;
    int ball;
    int bowler_id;
    int batsman_id;
    int mode;
    int innings;

    char bowler_name[50];
    char batsman_name[50];

} GanttCell;
extern GanttCell gantt_chart[MAX_BALL_EVENTS];
extern int gantt_count;

void record_gantt(int bowler,int batsman,int over,int ball);
void print_gantt_chart();

typedef struct {
    int runs;
    int wickets;
    int overs;
    int balls;
    int extras;
} Score;

typedef struct {
    int id;
    char name[50];
    char role[20];   // Batter / Bowler / All-Rounder / WK
    int runs;
    int balls_faced;
    int fours;
    int sixes;
    int is_out;
    int job_length;
    int arrival_time;
int start_time;
int wait_time;
int has_started;
} Batsman;

typedef struct {
    int id;
    char name[50];   // 🔥 added
    int balls_bowled;
    int runs_given;
    int wickets;
    int skill;
} Bowler;

typedef struct {
    Score score;
    int striker;
    int non_striker;
    int next_batsman;
    int ball_in_air;
    int match_running;
    int run_in_progress;
} MatchState;

typedef struct {
    Batsman players[MAX_BATSMEN];
    Bowler bowlers[MAX_BOWLERS];
    char name[50];
} Team;

extern Team team1;
extern Team team2;

extern MatchState match;

void update_score(int result);
void log_ball(int over, int ball, int result,
              int striker_id, int non_striker_id,
              int striker_before, int non_striker_before,
              int dismissal_type, int fielder_id,
              int was_free_hit);
void init_sync();
void swap_strike();

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
extern Batsman* batsmen;
extern Bowler* bowlers;

/* crease resources */
extern pthread_mutex_t end1_mutex;
extern pthread_mutex_t end2_mutex;

/* deadlock */
extern int striker_waiting;
extern int nonstriker_waiting;
extern pthread_mutex_t deadlock_mutex;

int attempt_run(int thread_id);
int detect_deadlock();
void resolve_deadlock();
void round_robin_scheduler();
void priority_scheduler();

void print_batsman_stats(Batsman* team, char* name);
void print_bowler_stats(Bowler* team, char* name);

extern int run_ready;
extern pthread_mutex_t run_mutex;
extern pthread_cond_t run_cond;

extern pthread_cond_t start_cond;

extern int target_score;
extern int innings;

extern int innings_started;

void print_team(Team team);
void perform_toss();

extern int toss_winner;
extern int toss_decision;

int generate_ball_event(Batsman* bat, Bowler* bowler);

extern int global_time;
int fcfs_scheduler();
extern int scheduling_type;

extern pthread_mutex_t start_mutex;

extern int rq_size;
extern int ready_queue[MAX_BATSMEN];

extern pthread_mutex_t crease_mutex;

int get_next_batsman();

extern int free_hit;

#define OUT_NONE     0
#define OUT_BOWLED   1
#define OUT_RUNOUT   2
#define OUT_DEADLOCK 3


#endif