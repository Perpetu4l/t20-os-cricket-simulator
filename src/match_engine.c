#include "../include/simulator.h"
#include <stdlib.h>

int generate_ball_event() {

    int r = rand() % 100;

    if(r < 5) return -1;      // wicket
    if(r < 25) return 0;      // dot
    if(r < 50) return 1;
    if(r < 65) return 2;
    if(r < 85) return 4;
    if(r < 95) return 6;

    return 7;                 // wide
}