#include "../include/simulator.h"
#include <time.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

MatchState match;
int pitch_ball = -2;
int free_hit=0;

int target_score = 0;
int innings = 1;

int global_time = 0;
int global_time_inning_1=0;
int scheduling_type = 0; // 0 = SJF, 1 = FCFS

float sjf_avg_team1, sjf_avg_team2;
float fcfs_avg_team1, fcfs_avg_team2;


char *first_team_name;
char *second_team_name;


int rq_size = 0;
int ready_queue[MAX_BATSMEN];

int innings_started = 0;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;

Team team1, team2;
Batsman *batsmen = NULL;
Bowler *bowlers = NULL;

void create_players();
void run_match(int mode);
void init_batsmen();
void init_bowlers();
void reset_match_state();
void reset_players();
float analyze_wait_time(Batsman *team, int time);
void print_wait_times(Batsman *team, char *name, int time);

int current_bowler = 0;
pthread_t bowler_threads[MAX_BOWLERS];

int main()
{

    srand(time(NULL));

    printf("+------------------------------------------------------+\n");
    printf("|     T20 WC 2026 Cricket Simulator  --  OS CSC-204   |\n");
    printf("+------------------------------------------------------+\n");
    printf("|  Threads: baad mei batayenge                          |\n");
    printf("|  Schedulers: RR + SJF + Priority                    |\n");
    printf("+------------------------------------------------------+\n\n");

    match.score = (Score){0, 0, 0, 0, 0};

    match.ball_in_air = 0;
    match.match_running = 1;


    match.run_in_progress = 0;

    strcpy(team1.name, "India");
    strcpy(team2.name, "New Zealand");

    init_sync();
    init_batsmen();
    init_bowlers();



    print_team(team1);
    print_team(team2);

    perform_toss();

    create_players();

    /* ================= SJF ================= */


    printf("\n\n========== SJF SIMULATION ==========\n");

    global_time = 0;
    global_time_inning_1=0;
    run_match(0);

    sjf_avg_team1 = analyze_wait_time(team1.players, global_time_inning_1);
    sjf_avg_team2 = analyze_wait_time(team2.players, global_time);
    print_wait_times(team1.players, team1.name, global_time_inning_1);
    print_wait_times(team2.players, team2.name, global_time);

    /* 🔥 STOP THREADS CLEANLY BEFORE NEXT RUN */
    pthread_mutex_lock(&start_mutex);
    innings_started = 0;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    usleep(1000); // give threads time to enter wait

    /* RESET EVERYTHING PROPERLY */
    reset_match_state();
    reset_players(); // 🔥 THIS WAS MISSING
    global_time = 0;

    pthread_mutex_lock(&run_mutex);
    run_ready = 1;

    pthread_cond_broadcast(&run_cond); // 🔥 WAKE ANY STUCK THREADS
    pthread_mutex_unlock(&run_mutex);

    /* ================= FCFS ================= */
    match.match_running = 1;
    // DO NOT reset sjf here
    printf("\n\n========== FCFS SIMULATION ==========\n");

    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    run_match(1);

    fcfs_avg_team1 = analyze_wait_time(team1.players, global_time_inning_1);
    fcfs_avg_team2 = analyze_wait_time(team2.players, global_time);

    print_wait_times(team1.players, team1.name, global_time_inning_1);
    print_wait_times(team2.players, team2.name, global_time);

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



    print_gantt_chart();

    printf("\nMatch Finished\n");
    match.match_running = 0;
    return 0;
}

void run_match(int mode)
{
    scheduling_type = mode;

    /* ================= INNINGS 1 ================= */

    innings = 1;
    if (toss_decision == 0)
    { // bat first
        if (toss_winner == 0)
        {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
        else
        {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
    }
    else
    { // bowl first
        if (toss_winner == 0)
        {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
        else
        {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
    }

    first_team_name = (batsmen == team1.players) ? team1.name : team2.name;
    // 🔥 RESET STRIKERS FOR THIS TEAM
    rq_size = 0;
    for(int i = 0; i < MAX_BATSMEN; i++){
        ready_queue[rq_size++] = i;
    }

    // scheduler decides openers
    match.striker = get_next_batsman();
    match.non_striker = get_next_batsman();

    global_time=0;
    
    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);
    pthread_mutex_lock(&batsman_mutex);
    pthread_cond_broadcast(&batsman_cond);
    pthread_mutex_unlock(&batsman_mutex);
    printf("\n==============================\n");
    printf("  INNINGS 1 START\n");
    printf("==============================\n\n");
    
    while (1)
    {
        if (match.score.overs >= MAX_OVERS)
            break;
        if (match.score.wickets >= 10)
            break;
        usleep(1000);
    }

    target_score = match.score.runs + 1;

    printf("\n--- END OF INNINGS 1 ---\n");
    printf("Score: %d/%d\n", match.score.runs, match.score.wickets);
    printf("Target: %d\n", target_score);

    global_time_inning_1=global_time;


    /* ================= STOP THREADS ================= */

    pthread_mutex_lock(&start_mutex);
    innings_started = 0;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);
   pthread_mutex_lock(&run_mutex);
    run_ready = 0;
    pthread_cond_broadcast(&run_cond);  // ← non-striker sees innings_started=0 and exits
    pthread_mutex_unlock(&run_mutex);
        // sleep(1)
    usleep(10000); // allow threads to pause

    /* ================= RESET ================= */

    reset_match_state();

    /* ================= INNINGS 2 ================= */

    global_time=0;


    innings = 2;
    if (toss_decision == 0)
    { // same order as innings 1
        if (toss_winner == 0)
        {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
        else
        {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
    }
    else
    {
        if (toss_winner == 0)
        {
            batsmen = team1.players;
            bowlers = team2.bowlers;
        }
        else
        {
            batsmen = team2.players;
            bowlers = team1.bowlers;
        }
    }

    second_team_name = (batsmen == team1.players) ? team1.name : team2.name;
    // 🔥 RESET STRIKERS FOR SECOND INNINGS TEAM
    rq_size = 0;
    for(int i = 0; i < MAX_BATSMEN; i++){
        ready_queue[rq_size++] = i;
    }

    // scheduler decides openers
    match.striker = get_next_batsman();
    match.non_striker = get_next_batsman();

    pthread_mutex_lock(&start_mutex);
    innings_started = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);
pthread_mutex_lock(&batsman_mutex);
pthread_cond_broadcast(&batsman_cond);
pthread_mutex_unlock(&batsman_mutex);
    printf("\n==============================\n");
    printf("  INNINGS 2 START\n");
    printf("==============================\n\n");

    while (1)
    {

        if (match.score.runs >= target_score)
        {
            printf("\nCHASE SUCCESSFUL!\n");
            break;
        }

        if (match.score.overs >= MAX_OVERS)
        {
            printf("\nOVERS COMPLETED!\n");
            break;

        }

        if (match.score.wickets >= 10)
        {
            printf("\nALL OUT!\n");
            break;
        }

        // sleep(1)
usleep(10000);
    }

    // match.match_running = 0;

    /* ================= RESULT ================= */

    printf("\n==============================\n");
    printf("        MATCH RESULT\n");
    printf("==============================\n");

    printf("%s: %d\n", first_team_name, target_score - 1);
    printf("%s: %d\n", second_team_name, match.score.runs);

    if (match.score.runs >= target_score)
    {
        printf("*** %s WON! ***\n", second_team_name);
    }
    else if (match.score.runs == target_score - 1)
    {
        printf("*** MATCH TIED! ***\n");
    }
    else
    {
        printf("*** %s WON! ***\n", first_team_name);
    }

    // TEAM 1 STATS
    print_batsman_stats(team1.players, team1.name);
    print_bowler_stats(team1.bowlers, team1.name);

    // TEAM 2 STATS
    print_batsman_stats(team2.players, team2.name);
    print_bowler_stats(team2.bowlers, team2.name);
}

/* ================= INIT BATSMEN ================= */

void init_batsmen()
{
        char *roles1[MAX_BATSMEN] = {
        "Batsman", "Batsman", "Batsman", "Batsman", "All-Rounder",
        "All-Rounder", "All-Rounder", "WK",
        "All-Rounder", "Bowler", "Bowler"
    };

    char *roles2[MAX_BATSMEN] = {
        "Batsman", "Batsman", "Batsman", "Batsman", "All-Rounder",
        "All-Rounder", "All-Rounder", "WK",
        "Bowler", "Bowler", "Bowler"
    };

    char *names1[MAX_BATSMEN] = {
        "Rohit Sharma", "Gill", "Virat Kohli", "SKY", "Krunal Pandya",
        "Hardik Pandya", "Jadeja", "Dhoni",
        "Axar", "Bumrah", "Siraj"
    };

    char *names2[MAX_BATSMEN] = {
        "Devon Conway", "Finn Allen", "Kane Williamson", "Daryl Mitchell", "Glenn Phillips",
        "James Neesham", "Santner", "Tim Seifert",
        "Trent Boult", "Tim Southee", "Lockie Ferguson"
    };

    /* Slightly different strengths for both teams */
    int job_lengths1[MAX_BATSMEN] = {50, 42, 40, 38, 34, 30, 26, 22, 18, 12, 8};
    int job_lengths2[MAX_BATSMEN] = {47, 41, 39, 36, 33, 29, 25, 21, 17, 11, 7};

    /* INIT LOOP */
    for (int i = 0; i < MAX_BATSMEN; i++)
    {
        // TEAM 1
        team1.players[i].id = i;
        strcpy(team1.players[i].name, names1[i]);
        strcpy(team1.players[i].role, roles1[i]);
        team1.players[i].runs = 0;
        team1.players[i].balls_faced = 0;
        team1.players[i].fours = 0;
        team1.players[i].sixes = 0;
        team1.players[i].is_out = 0;
        team1.players[i].job_length = job_lengths1[i];
        team1.players[i].turn_around_time = 1000;

team1.players[i].in_crease = 0;

        // TEAM 2
        team2.players[i].id = i;
        strcpy(team2.players[i].name, names2[i]);
        strcpy(team2.players[i].role, roles2[i]);
        team2.players[i].runs = 0;
        team2.players[i].balls_faced = 0;
        team2.players[i].fours = 0;
        team2.players[i].sixes = 0;
        team2.players[i].is_out = 0;
        team2.players[i].job_length =job_lengths2[i];
        team2.players[i].turn_around_time = 1000;

        team2.players[i].in_crease = 0;
    }

}

/* ================= INIT BOWLERS ================= */

void init_bowlers()
{

    char *names1[MAX_BOWLERS] = {
        "Siraj", "Shami", "Jadeja", "Hardik", "Krunal", "Bumrah"};

    char *names2[MAX_BOWLERS] = {
     "Southee", "Ferguson", "Santner", "Neesham", "Phillips", "Boult"
    };

    int bowler_skills1[MAX_BOWLERS] = {40, 38, 35, 32, 32, 45};
    int bowler_skills2[MAX_BOWLERS] = {39, 37, 34, 30, 30, 45};

    for (int i = 0; i < MAX_BOWLERS; i++)
    {
        team1.bowlers[i].id = i;
        strcpy(team1.bowlers[i].name, names1[i]);
        team1.bowlers[i].balls_bowled = 0;
        team1.bowlers[i].runs_given = 0;
        team1.bowlers[i].wickets = 0;
        team1.bowlers[i].skill = bowler_skills1[i];
    }

    for (int i = 0; i < MAX_BOWLERS; i++)
    {
        team2.bowlers[i].id = i;
        strcpy(team2.bowlers[i].name, names2[i]);
        team2.bowlers[i].balls_bowled = 0;
        team2.bowlers[i].runs_given = 0;
        team2.bowlers[i].wickets = 0;
        team2.bowlers[i].skill = bowler_skills2[i];
    }
}

/* ================= RESET ================= */

void reset_match_state()
{

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

}

float analyze_wait_time(Batsman *team, int time)
{

    int total = 0, count = 0;

    for (int i = 3; i <= 6; i++)
    {
        total += min(team[i].turn_around_time,time)-team[i].balls_faced;
        count++;
    }

    return (float)total / count;
}

void reset_players()
{

    for (int i = 0; i < MAX_BATSMEN; i++)
    {

        // TEAM 1
        team1.players[i].runs = 0;
        team1.players[i].balls_faced = 0;
        team1.players[i].fours = 0;
        team1.players[i].sixes = 0;
        team1.players[i].is_out = 0;
        team1.players[i].turn_around_time = 1000;


        team1.players[i].in_crease = 0;
        // TEAM 2
        team2.players[i].runs = 0;
        team2.players[i].balls_faced = 0;
        team2.players[i].fours = 0;
        team2.players[i].sixes = 0;
        team2.players[i].turn_around_time = 1000;
        team2.players[i].is_out = 0;


        team2.players[i].in_crease = 0;
    }

    for (int i = 0; i < MAX_BOWLERS; i++)
    {

        team1.bowlers[i].balls_bowled = 0;
        team1.bowlers[i].runs_given = 0;
        team1.bowlers[i].wickets = 0;

        team2.bowlers[i].balls_bowled = 0;
        team2.bowlers[i].runs_given = 0;
        team2.bowlers[i].wickets = 0;
    }
}

void print_wait_times(Batsman *team, char *name, int time)
{

    printf("\n=== WAIT TIME ANALYSIS (%s) ===\n", name);

    printf("------------------------------------------------------------------\n");
    printf("%-15s | %-18s | %-13s | %-10s\n",
        "Player Name", "Turn Around Time", "Burst Time", "Wait Time");
    printf("------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_BATSMEN; i++)
    {
        printf("%-15s | %-18d | %-13d | %-10d\n",
            team[i].name,
            min(team[i].turn_around_time,time),
            team[i].balls_faced,
            min(team[i].turn_around_time,time)-team[i].balls_faced);
    }

    printf("------------------------------------------------------------------\n");

    printf("\n[MIDDLE ORDER AVG (3-6)] = %.2f\n",
           analyze_wait_time(team, time));
}
