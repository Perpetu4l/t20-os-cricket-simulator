#include "../include/simulator.h"
#include <stdio.h>

void print_batsman_stats(Batsman* team, char* team_name){

    printf("\n=== %s BATTING STATS ===\n", team_name);
    printf("%-15s %5s %5s %5s %5s %8s\n","Name","Runs","Balls","4s","6s","SR");

    int printed = 0;

    for(int i=0;i<11;i++){
        if(team[i].runs == 0 &&team[i].balls_faced == 0 &&team[i].is_out == 0) continue;

        double sr = 0.0;
        if(team[i].balls_faced > 0){
            sr = (team[i].runs * 100.0) / team[i].balls_faced;
        }

        printf("%-15s %5d %5d %5d %5d %8.2f\n",team[i].name,team[i].runs,team[i].balls_faced,team[i].fours, team[i].sixes,sr);

        printed = 1;
    }

    if(!printed){
        printf("No batting data.\n");
    }
}

void print_bowler_stats(Bowler* team, char* team_name){

    printf("\n=== %s BOWLING STATS ===\n", team_name);
    printf("%-15s %7s %7s %7s %8s\n","Name","Overs","Runs","Wkts","Econ");

    int printed = 0;

    for(int i=0;i<6;i++){

        int balls = team[i].balls_bowled;
        if(balls == 0 &&team[i].runs_given == 0 &&team[i].wickets == 0) continue;

        int ov = balls / 6;
        int rem = balls % 6;

        double econ = 0.0;
        if(balls > 0){
            econ = team[i].runs_given / (balls / 6.0);
        }

        printf("%-15s %2d.%d     %5d   %5d   %6.2f\n",team[i].name,ov, rem,team[i].runs_given,team[i].wickets,econ);

        printed = 1;
    }

    if(!printed){
        printf("No bowling data.\n");
    }
}