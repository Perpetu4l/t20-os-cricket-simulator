#include "../include/simulator.h"
#include <stdio.h>
#include <string.h>

GanttCell gantt_chart[MAX_BALL_EVENTS];
int gantt_count = 0;

extern int innings;
extern int scheduling_type;
extern Batsman* batsmen;
extern Bowler* bowlers;

char* get_result_str(int r)
{
    if (r == -1) return "OUT";
    if (r == 0)  return "DOT";
    if (r == 1)  return "1";
    if (r == 2)  return "2";
    if (r == 3)  return "3";
    if (r == 4)  return "FOUR";
    if (r == 6)  return "SIX";
    if (r == 7)  return "WIDE";
    if (r == 8)  return "NO BALL";
    return "?";
}

char* get_dismissal_str(int d)
{
    if (d == OUT_NONE)     return "-";
    if (d == OUT_BOWLED)   return "BOWLED";
    if (d == OUT_RUNOUT)   return "RUN OUT";
    if (d == OUT_DEADLOCK) return "DEADLOCK";
    return "?";
}

void record_gantt(int bowler, int batsman, int over, int ball,int result, int dismissal_type)
{
    // ALWAYS check bounds first (you were writing out of bounds before 💀)
    if (gantt_count >= MAX_BALL_EVENTS)
        return;

    gantt_chart[gantt_count].bowler_id = bowler;
    gantt_chart[gantt_count].batsman_id = batsman;
    gantt_chart[gantt_count].over = over;
    gantt_chart[gantt_count].ball = ball;

    gantt_chart[gantt_count].mode = scheduling_type;
    gantt_chart[gantt_count].innings = innings;

    // store names (this is the whole point)
    strcpy(gantt_chart[gantt_count].bowler_name,
           bowlers[bowler].name);

    strcpy(gantt_chart[gantt_count].batsman_name,
           batsmen[batsman].name);

    gantt_chart[gantt_count].result = result;
    gantt_chart[gantt_count].dismissal_type = dismissal_type;

    gantt_count++;
}

void print_gantt_chart()
{
    printf("\n==================== GANTT CHART ====================\n");

    int last_mode = -1;
    int last_innings = -1;

    for (int i = 0; i < gantt_count; i++)
    {

        if (gantt_chart[i].mode != last_mode)
        {
            printf("\n====================================================\n");
            printf("                 %s SIMULATION\n",
                   gantt_chart[i].mode == 0 ? "SJF" : "FCFS");
            printf("====================================================\n");

            last_mode = gantt_chart[i].mode;
            last_innings = -1;
        }

        if (gantt_chart[i].innings != last_innings)
        {
            printf("\n[ INNINGS %d ]\n\n", gantt_chart[i].innings);

            printf("%-8s %-18s %-18s %-10s %-12s\n",
                   "Ball", "Bowler", "Batsman", "Result", "Dismissal");

            printf("--------------------------------------------------------------------------\n");

            last_innings = gantt_chart[i].innings;
        }

        char ball_str[10];
        snprintf(ball_str, sizeof(ball_str), "%d.%d",
                 gantt_chart[i].over,
                 gantt_chart[i].ball);

        printf("%-8s %-18s %-18s %-10s %-12s\n",
               ball_str,
               gantt_chart[i].bowler_name,
               gantt_chart[i].batsman_name,
               get_result_str(gantt_chart[i].result),
               get_dismissal_str(gantt_chart[i].dismissal_type));
    }

    printf("\n");
}