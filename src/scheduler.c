#include "../include/simulator.h"
void round_robin_scheduler(){
    int prev = current_bowler;

    current_bowler = (current_bowler + 1) % 6;

    pthread_mutex_lock(&print_mutex);

    int balls = bowlers[prev].balls_bowled;
    int ov = balls / 6;
    int rem = balls % 6;

    double economy = 0.0;
    if(balls > 0){
        economy = bowlers[prev].runs_given / (balls / 6.0);
    }

    printf("\n  [RR CTX SWITCH] End of over %d | %s: %d.%d ov %d R %d W Econ: %.2f\n",match.score.overs,bowlers[prev].name,ov, rem,bowlers[prev].runs_given,bowlers[prev].wickets,economy);

    printf("  [RR CTX SWITCH] Loading %s for over %d...\n\n",bowlers[current_bowler].name,match.score.overs + 1);

    pthread_mutex_unlock(&print_mutex);
}


void priority_scheduler(){
 
    if(match.score.overs == 19){

        int prev = current_bowler;

        int balls = bowlers[prev].balls_bowled;
        int ov = balls / 6;
        int rem = balls % 6;

        double economy = 0.0;
        if(balls > 0){
            economy = bowlers[prev].runs_given / (balls / 6.0);
        }

        current_bowler = 5;

        pthread_mutex_lock(&print_mutex);

        printf("\n  [PRIORITY!! CTX SWITCH : DEATH OVER] End of over %d | %s: %d.%d ov %d R %d W Econ: %.2f\n",match.score.overs, bowlers[prev].name, ov, rem,bowlers[prev].runs_given,bowlers[prev].wickets,economy);

        printf("  [PRIORITY!! CTX SWITCH : DEATH OVER] Loading %s for over %d...\n\n",bowlers[current_bowler].name,match.score.overs + 1 );
        pthread_mutex_unlock(&print_mutex);
    }
}


int get_next_batsman()
{
    if(rq_size == 0) return -1;

    int idx = 0;

    if(scheduling_type == 0)  // if its sjf this path is followed to get smallest job length index
    {
        for(int i = 1; i < rq_size; i++)
        {
            if(batsmen[ready_queue[i]].job_length <
               batsmen[ready_queue[idx]].job_length)
                idx = i;
        }
    }
    int chosen = ready_queue[idx];
    for(int i = idx; i < rq_size - 1; i++)
        ready_queue[i] = ready_queue[i + 1];

    rq_size--;

    return chosen;
}