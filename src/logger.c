#include "../include/simulator.h"
#include <string.h>

void log_ball(int over, int ball, int result, int batsman_id) {

    pthread_mutex_lock(&print_mutex);

    char *batsman = batsmen[batsman_id].name;
    char *bowler  = bowlers[current_bowler].name;

    char event[20];

    if(result == -1) strcpy(event, "WICKET!");
    else if(result == 0) strcpy(event, "dot");
    else if(result == 1) strcpy(event, "1 run");
    else if(result == 2) strcpy(event, "2 runs");
    else if(result == 3) strcpy(event, "3 runs");
    else if(result == 4) strcpy(event, "FOUR!");
    else if(result == 6) strcpy(event, "SIX!!");
    else if(result == 7) strcpy(event, "Wide");
    else if(result == 8) strcpy(event, "NO BALL");

    
    printf("  Over %2d.%d  %-15s to %-15s %-8s  %3d/%d\n",
        over,
        ball,
        bowler,
        batsman,
        event,
        match.score.runs,
        match.score.wickets
    );
    if(innings == 2){
        printf("  (Target: %d)", target_score);
    }
    printf("\n");

    pthread_mutex_unlock(&print_mutex);
}