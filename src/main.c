#include "../include/simulator.h"

MatchState match;

void create_players();
void run_match();

int main() {

    printf("Starting T20 Cricket Simulator\n");

    match.score.runs = 0;
    match.score.wickets = 0;
    match.score.overs = 0;
    match.score.balls = 0;

    match.match_running = 1;

    create_players();

    run_match();

    printf("Match Finished\n");

    return 0;
}

void run_match() {

    while(match.score.overs < MAX_OVERS) {

        printf("Over %d.%d\n",
        match.score.overs + 1,
        match.score.balls + 1);

        sleep(1);

        match.score.balls++;

        if(match.score.balls == 6) {
            match.score.balls = 0;
            match.score.overs++;
        }
    }

    match.match_running = 0;
}