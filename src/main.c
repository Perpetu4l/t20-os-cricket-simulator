#include "../include/simulator.h"
#include <time.h>

MatchState match;
int pitch_ball = -2;

int target_score = 0;
int innings = 1;

int global_time = 0;
int scheduling_type = 0; // 0 = SJF, 1 = FCFS

float sjf_avg_team1, sjf_avg_team2;
float fcfs_avg_team1, fcfs_avg_team2;


int innings_started = 0;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;

Team team1, team2;
Batsman* batsmen = NULL;
Bowler* bowlers = NULL;

void create_players();
void run_match(int mode);
void init_batsmen();
void init_bowlers();
void reset_match_state();
void reset_players();
float analyze_wait_time(Batsman* team);
void print_wait_times(Batsman* team, char* name);

int current_bowler = 0;
pthread_t bowler_threads[MAX_BOWLERS];

int main() {

    srand(time(NULL));

    printf("+------------------------------------------------------+\n");
    printf("|     T20 WC 2026 Cricket Simulator  --  OS CSC-204   |\n");
    printf("+------------------------------------------------------+\n");
    printf("|  Threads: baad mei batayenge                          |\n");
    printf("|  Schedulers: RR + SJF + Priority                    |\n");
    printf("+------------------------------------------------------+\n\n");

    match.score = (Score){0,0,0,0,0};

    match.ball_in_air = 0;
    match.match_running = 1;

    match.striker = 0;
    match.non_striker = 1;
    match.next_batsman = 2;

    match.run_in_progress = 0;

    strcpy(team1.name, "India");
    strcpy(team2.name, "Australia");

    init_sync();
    init_batsmen();
    init_bowlers();

    /* 🔥 SET INITIAL ARRIVAL TIME (OPENERS) */
    team1.players[0].arrival_time = 0;
    team1.players[1].arrival_time = 0;

    team2.players[0].arrival_time = 0;
    team2.players[1].arrival_time = 0;

    print_team(team1);
    print_team(team2);

    perform_toss();

    create_players();

    /* ================= SJF ================= */

    printf("\n\n========== SJF SIMULATION ==========\n");

    global_time = 0;
    run_match(0);

    sjf_avg_team1 = analyze_wait_time(team1.players);
    sjf_avg_team2 = analyze_wait_time(team2.players);

    print_wait_times(team1.players, team1.name);
    print_wait_times(team2.players, team2.name);

    /* 🔥 STOP THREADS CLEANLY BEFORE NEXT RUN */
    pthread_mutex_lock(&start_mutex);
    innings_started = 0;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    sleep(1);  // give threads time to enter wait

    /* RESET EVERYTHING PROPERLY */
    reset_match_state();
    reset_players();   // 🔥 THIS WAS MISSING
    global_time = 0;

    pthread_mutex_lock(&run_mutex);
    run_ready = 0;

    pthread_cond_broadcast(&run_cond);  // 🔥 WAKE ANY STUCK THREADS
    pthread_mutex_unlock(&run_mutex);

    /* ================= FCFS ================= */
    match.match_running = 1;
    printf("\n\n========== FCFS SIMULATION ==========\n");

    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    run_match(1);

    fcfs_avg_team1 = analyze_wait_time(team1.players);
    fcfs_avg_team2 = analyze_wait_time(team2.players);

    print_wait_times(team1.players, team1.name);
    print_wait_times(team2.players, team2.name);

        /* 🔥 FINAL COMPARISON */
    printf("\n\n====================================\n");
    printf("        FINAL COMPARISON\n");
    printf("====================================\n");

    printf("\nTeam: %s\n", team1.name);
    printf("SJF Avg Wait Time  = %.2f\n", sjf_avg_team1);
    printf("FCFS Avg Wait Time = %.2f\n", fcfs_avg_team1);

    printf("\nTeam: %s\n", team2.name);
    printf("SJF Avg Wait Time  = %.2f\n", sjf_avg_team2);
    printf("FCFS Avg Wait Time = %.2f\n", fcfs_avg_team2);

    printf("\n====================================\n");

    if(sjf_avg_team1 < fcfs_avg_team1)
        printf("SJF performed better for %s\n", team1.name);
    else
        printf("FCFS performed better for %s\n", team1.name);

    if(sjf_avg_team2 < fcfs_avg_team2)
        printf("SJF performed better for %s\n", team2.name);
    else
    printf("FCFS performed better for %s\n", team2.name);

    print_gantt_chart();

    printf("\nMatch Finished\n");
    match.match_running = 0;
    return 0;
}

void run_match(int mode){
    scheduling_type = mode;

    /* ================= INNINGS 1 ================= */

    innings = 1;
    if(toss_decision == 0){ // bat first
        if(toss_winner == 0){
            batsmen = team1.players;
            bowlers = team2.bowlers;
        } else {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
    } else { // bowl first
        if(toss_winner == 0){
            batsmen = team2.players;
            bowlers = team1.bowlers;
        } else {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
    }
    // 🔥 RESET STRIKERS FOR THIS TEAM
match.striker = 0;
match.non_striker = 1;
match.next_batsman = 2;



    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    printf("\n==============================\n");
    printf("  INNINGS 1 START\n");
    printf("==============================\n\n");

    while(1){
        if(match.score.overs >= MAX_OVERS) break;
        if(match.score.wickets >= 10) break;
        sleep(1);
    }

    target_score = match.score.runs + 1;

    printf("\n--- END OF INNINGS 1 ---\n");
    printf("Score: %d/%d\n", match.score.runs, match.score.wickets);
    printf("Target: %d\n", target_score);

    /* ================= STOP THREADS ================= */

    pthread_mutex_lock(&start_mutex);
    innings_started = 0;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    sleep(1); // allow threads to pause

    /* ================= RESET ================= */

    reset_match_state();

    /* ================= INNINGS 2 ================= */

    innings = 2;
    if(toss_decision == 0){ // same order as innings 1
        if(toss_winner == 0){
            batsmen = team2.players;
            bowlers = team1.bowlers;
        } else {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
    } else {
        if(toss_winner == 0){
            batsmen = team1.players;
            bowlers = team2.bowlers;
        } else {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
    }
    // 🔥 RESET STRIKERS FOR SECOND INNINGS TEAM
match.striker = 0;
match.non_striker = 1;
match.next_batsman = 2;


    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    printf("\n==============================\n");
    printf("  INNINGS 2 START\n");
    printf("==============================\n\n");

    while(1){

        if(match.score.runs >= target_score){
            printf("\nCHASE SUCCESSFUL!\n");
            break;
        }

        if(match.score.overs >= MAX_OVERS){
            printf("\nOVERS COMPLETED!\n");
            break;
        }

        if(match.score.wickets >= 10){
            printf("\nALL OUT!\n");
            break;
        }

        sleep(1);
    }

    // match.match_running = 0;

    /* ================= RESULT ================= */

    printf("\n==============================\n");
    printf("        MATCH RESULT\n");
    printf("==============================\n");

    printf("%s: %d\n", team1.name, target_score-1);
    printf("%s: %d\n", team2.name, match.score.runs);

    if(match.score.runs >= target_score){
        printf("*** %s WON! ***\n", team2.name);
    }
    else{
        printf("*** %s WON! ***\n", team1.name);
    }

    // TEAM 1 STATS
    print_batsman_stats(team1.players, team1.name);
    print_bowler_stats(team1.bowlers, team1.name);

    // TEAM 2 STATS
    print_batsman_stats(team2.players, team2.name);
    print_bowler_stats(team2.bowlers, team2.name);



}



/* ================= INIT BATSMEN ================= */

void init_batsmen(){

    char* roles1[MAX_BATSMEN] = {
    "Batsman","Batsman","Batsman","Batsman","WK",
    "All-Rounder","All-Rounder","WK",
    "All-Rounder","Bowler","Bowler"
    };

    char* roles2[MAX_BATSMEN] = {
        "Batsman","Batsman","Batsman","Batsman","All-Rounder",
        "All-Rounder","WK","Bowler",
        "Bowler","Bowler","Bowler"
    };

    char* names1[MAX_BATSMEN] = {
        "Rohit Sharma","Gill","Virat Kohli","SKY","KL Rahul",
        "Hardik Pandya","Jadeja","Dhoni",
        "Axar","Bumrah","Siraj"
    };

    char* names2[MAX_BATSMEN] = {
        "Warner","Head","Smith","Labuschagne","Maxwell",
        "Stoinis","Carey","Cummins",
        "Starc","Hazlewood","Zampa"
    };

    int job_lengths[MAX_BATSMEN] =
    {50,40,30,25,20,15,12,10,7,5,3};

    for(int i=0;i<MAX_BATSMEN;i++){
        team1.players[i].id = i;
        strcpy(team1.players[i].name, names1[i]);
        strcpy(team1.players[i].role, roles1[i]);
        team1.players[i].runs = 0;
        team1.players[i].balls_faced = 0;
        team1.players[i].fours = 0;
        team1.players[i].sixes = 0;
        team1.players[i].is_out = 0;
        team1.players[i].job_length = job_lengths[i];
        team1.players[i].arrival_time = 0;
        team1.players[i].start_time = -1;
        team1.players[i].wait_time = 0;
        team1.players[i].has_started = 0;
    }

    for(int i=0;i<MAX_BATSMEN;i++){
        team2.players[i].id = i;
        strcpy(team2.players[i].name, names2[i]);
        strcpy(team2.players[i].role, roles2[i]);
        team2.players[i].runs = 0;
        team2.players[i].balls_faced = 0;
        team2.players[i].fours = 0;
        team2.players[i].sixes = 0;
        team2.players[i].is_out = 0;
        team2.players[i].job_length = job_lengths[i];
        team2.players[i].arrival_time = 0;
        team2.players[i].start_time = -1;
        team2.players[i].wait_time = 0;
        team2.players[i].has_started = 0;
    }
}

/* ================= INIT BOWLERS ================= */

void init_bowlers(){

    char* names1[MAX_BOWLERS] = {
        "Bumrah","Siraj","Shami","Jadeja","Hardik"
    };

    char* names2[MAX_BOWLERS] = {
        "Cummins","Starc","Hazlewood","Zampa","Stoinis"
    };

    for(int i=0;i<MAX_BOWLERS;i++){
        team1.bowlers[i].id = i;
        strcpy(team1.bowlers[i].name, names1[i]);
        team1.bowlers[i].balls_bowled = 0;
        team1.bowlers[i].runs_given = 0;
        team1.bowlers[i].wickets = 0;
    }

    for(int i=0;i<MAX_BOWLERS;i++){
        team2.bowlers[i].id = i;
        strcpy(team2.bowlers[i].name, names2[i]);
        team2.bowlers[i].balls_bowled = 0;
        team2.bowlers[i].runs_given = 0;
        team2.bowlers[i].wickets = 0;
    }
}

/* ================= RESET ================= */

void reset_match_state(){

    pthread_mutex_lock(&pitch_mutex);

    match.score.runs = 0;
    match.score.wickets = 0;
    match.score.overs = 0;
    match.score.balls = 0;

    match.striker = 0;
    match.non_striker = 1;
    match.next_batsman = 2;

    match.run_in_progress = 0;
    match.ball_in_air = 0;

    pitch_ball = -2;
    current_bowler = 0;

    pthread_mutex_unlock(&pitch_mutex);

    pthread_mutex_lock(&run_mutex);
    run_ready = 0;
    pthread_mutex_unlock(&run_mutex);

    pthread_mutex_lock(&deadlock_mutex);
    striker_waiting = 0;
    nonstriker_waiting = 0;
    pthread_mutex_unlock(&deadlock_mutex);

    for(int i=0;i<MAX_BATSMEN;i++){
        batsmen[i].start_time = -1;
        batsmen[i].wait_time = 0;
        batsmen[i].has_started = 0;
    }

}

float analyze_wait_time(Batsman* team){

    int total = 0, count = 0;

    for(int i = 3; i <= 6; i++){
        total += team[i].wait_time;
        count++;
    }

    return (float)total / count;
}

void reset_players(){
    
    for(int i=0;i<MAX_BATSMEN;i++){

        // TEAM 1
        team1.players[i].runs = 0;
        team1.players[i].balls_faced = 0;
        team1.players[i].fours = 0;
        team1.players[i].sixes = 0;
        team1.players[i].is_out = 0;

        team1.players[i].arrival_time = (i < 2 ? 0 : -1);
        team1.players[i].start_time = -1;
        team1.players[i].wait_time = 0;
        team1.players[i].has_started = 0;

        // TEAM 2
        team2.players[i].runs = 0;
        team2.players[i].balls_faced = 0;
        team2.players[i].fours = 0;
        team2.players[i].sixes = 0;
        team2.players[i].is_out = 0;

        team2.players[i].arrival_time = (i < 2 ? 0 : -1);
        team2.players[i].start_time = -1;
        team2.players[i].wait_time = 0;
        team2.players[i].has_started = 0;
    }

    for(int i=0;i<MAX_BOWLERS;i++){

        team1.bowlers[i].balls_bowled = 0;
        team1.bowlers[i].runs_given = 0;
        team1.bowlers[i].wickets = 0;

        team2.bowlers[i].balls_bowled = 0;
        team2.bowlers[i].runs_given = 0;
        team2.bowlers[i].wickets = 0;
    }
}

void print_wait_times(Batsman* team, char* name){

    printf("\n=== WAIT TIME ANALYSIS (%s) ===\n", name);

    for(int i=0;i<MAX_BATSMEN;i++){
        printf("%-15s -> %d\n", team[i].name, team[i].wait_time);
    }

    printf("\n[MIDDLE ORDER AVG (3-6)] = %.2f\n",
           analyze_wait_time(team));
}