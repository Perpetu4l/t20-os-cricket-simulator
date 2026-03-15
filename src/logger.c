#include "../include/simulator.h"
#include <stdio.h>

void log_ball(int over, int ball, int result) {

    int display_over = over - 1;
    int display_ball = ball;

    if(ball == 6){
        display_over += 1;
        display_ball = 0;
    }

    pthread_mutex_lock(&print_mutex);

    if(result == -1)
        printf("Over %d.%d -> WICKET\n", display_over, display_ball);

    else if(result == 7)
        printf("Over %d.%d -> WIDE\n", display_over, display_ball);

    else
        printf("Over %d.%d -> %d runs\n", display_over, display_ball, result);

    printf("Score: %d/%d\n",
        match.score.runs,
        match.score.wickets);
    printf("--------------------------------\n");

    pthread_mutex_unlock(&print_mutex);
}