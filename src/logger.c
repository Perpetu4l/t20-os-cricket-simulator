#include "../include/simulator.h"
#include <stdio.h>

void log_ball(int over, int ball, int result) {

    if(result == -1)
        printf("Over %d.%d -> WICKET\n", over, ball);

    else if(result == 7)
        printf("Over %d.%d -> WIDE\n", over, ball);

    else
        printf("Over %d.%d -> %d runs\n", over, ball, result);

    printf("Score: %d/%d\n",
        match.score.runs,
        match.score.wickets);
}