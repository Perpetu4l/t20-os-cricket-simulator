#include "../include/simulator.h"

MatchState match;
int pitch_ball = -2;

void create_players();
void run_match();

int main() {
    
    printf("Starting T20 Cricket Simulator\n");
    
    match.score.runs = 0;
    match.score.wickets = 0;
    match.score.overs = 0;
    match.score.balls = 0;
    
    match.match_running = 1;
    
    init_sync();
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