#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_FIELDERS 10
#define MAX_BATSMEN 11
#define MAX_BOWLERS 5
#define MAX_OVERS 20

typedef struct {
    int runs;
    int wickets;
    int overs;
    int balls;
} Scoreboard;

typedef struct {
    int id;
    int runs;
    int balls_faced;
    int is_out;
} Batsman;

typedef struct {
    int id;
    int balls_bowled;
    int runs_given;
    int wickets;
} Bowler;

typedef struct {
    Scoreboard score;
    int match_running;
} MatchState;

extern MatchState match;

#endif