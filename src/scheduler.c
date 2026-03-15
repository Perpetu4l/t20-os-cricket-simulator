#include "../include/simulator.h"


void round_robin_scheduler(){

    int prev = current_bowler;

    current_bowler =
        (current_bowler + 1) % MAX_BOWLERS;

    pthread_mutex_lock(&print_mutex);

    printf("\n---- Context Switch ----\n");
    printf("Saving Bowler %d stats:\n", prev);
    printf("Balls: %d  Runs: %d  Wickets: %d\n",
        bowlers[prev].balls_bowled,
        bowlers[prev].runs_given,
        bowlers[prev].wickets);

    printf("Loading Bowler %d\n\n", current_bowler);

    pthread_mutex_unlock(&print_mutex);
}

int sjf_scheduler(){

    int best = -1;
    int shortest = 100000;

    for(int i = 0; i < MAX_BATSMEN; i++){

        if(batsmen[i].is_out == 0 &&
           batsmen[i].job_length < shortest){

            shortest = batsmen[i].job_length;
            best = i;
        }
    }

    return best;
}

// int sjf_scheduler(){

//     int best = -1;
//     int shortest = 100000;

//     for(int i = 0; i < MAX_BATSMEN; i++){

//         if(batsmen[i].is_out == 0 &&
//            batsmen[i].job_length < shortest){

//             shortest = batsmen[i].job_length;
//             best = i;
//         }
//     }

//     if(best != -1){
//         batsmen[best].is_out = 1;   // mark as already used
//     }

//     return best;
// }

void priority_scheduler(){// realsitci banan he 

    if(match.score.overs >= 19){

        current_bowler = DEATH_OVER_BOWLER;

        pthread_mutex_lock(&print_mutex);

        printf("\n*** Priority Scheduler: Death Over Specialist Bowler %d ***\n\n",
               current_bowler);

        pthread_mutex_unlock(&print_mutex);
    }
}