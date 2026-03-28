#include "../include/simulator.h"
#include <string.h>

void log_ball(int over, int ball, int result,
              int striker_id, int non_striker_id,
              int striker_before, int non_striker_before,
              int dismissal_type, int fielder_id,
              int was_free_hit, int victim)
{
    pthread_mutex_lock(&print_mutex);

    char *striker     = batsmen[striker_id].name;
    char *striker_before_name= batsmen[striker_before].name;
    char *non_striker = batsmen[non_striker_id].name;
    char *bowler      = bowlers[current_bowler].name;

    char event[80];

    // 🎯 EVENT BUILDING
    if (result == -1)
    {
        if (was_free_hit && dismissal_type == OUT_BOWLED)
        {
            strcpy(event, "BOWLED (Free Hit - Not Out)");
        }
        else if (dismissal_type == OUT_RUNOUT)
        {
            sprintf(event, "RUN OUT (Fielder %d)", fielder_id);
        }
        else if (dismissal_type == OUT_DEADLOCK)
        {
            strcpy(event, "RUN OUT (Mix-up!)");
        }
        else
        {
            sprintf(event, "BOWLED by %s", bowler);
        }
    }
    else if(result == 0) strcpy(event, "dot");
    else if(result == 1) strcpy(event, "1 run");
    else if(result == 2) strcpy(event, "2 runs");
    else if(result == 3) strcpy(event, "3 runs");
    else if(result == 4) strcpy(event, "FOUR!");
    else if(result == 6) strcpy(event, "SIX!!");
    else if(result == 7) strcpy(event, "Wide");
    else if(result == 8) strcpy(event, "NO BALL + Free Hit");

    // 🎯 MAIN LINE
    
    printf("Over %2d.%d | %-12s → %-15s | %-22s | %3d/%d\n",
        over, ball,
        bowler,
        striker_before_name,
        event,
        match.score.runs,
        match.score.wickets
    );
 

    // 👉 ADD THIS
    if (was_free_hit)
        printf("  [FREE HIT]");
        
    printf("\n");

    // 🎯 STRIKER INFO LINE (THIS IS THE SEXY PART)
    if (match.score.wickets < 10){
        if(ball!=0||(ball==0&&(result==7||result==8))){
            printf("        %s* %d(%d)   |   %s %d(%d)\n",
                striker,
                batsmen[striker_id].runs,
                batsmen[striker_id].balls_faced,
                non_striker,
                batsmen[non_striker_id].runs,
                batsmen[non_striker_id].balls_faced
            );
        }
        else{
            printf("        %s* %d(%d)   |   %s %d(%d)\n",
                non_striker,
                batsmen[non_striker_id].runs,
                batsmen[non_striker_id].balls_faced,
                striker,
                batsmen[striker_id].runs,
                batsmen[striker_id].balls_faced
            );
        }
    }

    // 🎯 WICKET CARD
    if (result == -1 && !(was_free_hit && dismissal_type == OUT_BOWLED))
    {
        printf("        %s %d(%d)  ",
            batsmen[victim].name,
            batsmen[victim].runs,
            batsmen[victim].balls_faced
        );

        if (dismissal_type == OUT_RUNOUT)
            printf("run out (Fielder %d)\n", fielder_id);
        else if (dismissal_type == OUT_DEADLOCK)
            printf("run out (mix-up)\n");
        else
            printf("b %s\n", bowler);
    }

    // 🎯 TARGET INFO
    if (innings == 2)
    {
        int need = target_score - match.score.runs;
        printf("        Need: %d runs\n", need > 0 ? need : 0);
    }

    printf("\n");

    pthread_mutex_unlock(&print_mutex);
}