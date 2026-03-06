#include "../include/simulator.h"

MatchState match;
int pitch_ball = -2;

void create_players();
void run_match();
void init_batsmen();   // <-- add prototype

Batsman batsmen[MAX_BATSMEN];

int current_bowler = 0;
pthread_t bowler_threads[MAX_BOWLERS];

int main() {

    srand(time(NULL));

    printf("Starting T20 Cricket Simulator\n");

    match.score.runs = 0;
    match.score.wickets = 0;
    match.score.overs = 0;
    match.score.balls = 0;

    match.ball_in_air = 0;
    match.match_running = 1;

    init_sync();

    init_batsmen();   // <-- IMPORTANT: initialize batting lineup

    create_players();

    run_match();

    printf("Match Finished\n");

    return 0;
}

void run_match() {

    while(match.score.overs < MAX_OVERS) {
        sleep(1);

        if(match.score.wickets >= 10)
            break;
    }

    match.match_running = 0;
}

void init_batsmen(){

    int job_lengths[MAX_BATSMEN] =
    {50,40,30,25,20,15,12,10,7,5,3};

    for(int i=0;i<MAX_BATSMEN;i++){
        batsmen[i].id = i;
        batsmen[i].runs = 0;
        batsmen[i].balls_faced = 0;
        batsmen[i].is_out = 0;
        batsmen[i].job_length = job_lengths[i];
    }
}