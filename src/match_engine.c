#include "../include/simulator.h"
#include <stdlib.h>
#include <time.h>

int generate_ball_event() {

    int r = rand() % 100;

    if(r < 5) return -1;       // wicket
    if(r < 25) return 0;       // dot ball
    if(r < 50) return 1;       // single
    if(r < 65) return 2;       // double
    if(r < 85) return 4;       // four
    if(r < 95) return 6;       // six

    return 7;                  // wide
}

void update_score(int result) {

    if(result == -1) {
        match.score.wickets++;
        printf("WICKET!\n");
        return;
    }

    if(result == 7) {
        match.score.runs += 1;
        printf("WIDE BALL\n");
        return;
    }

    match.score.runs += result;
}