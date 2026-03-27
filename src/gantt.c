#include "../include/simulator.h"
#include <stdio.h>
#include <string.h>

GanttCell gantt_chart[MAX_BALL_EVENTS];
int gantt_count = 0;

extern int innings;
extern int scheduling_type;
extern Batsman* batsmen;
extern Bowler* bowlers;

void record_gantt(int bowler, int batsman, int over, int ball)
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

    gantt_count++;
}

void print_gantt_chart()
{
    printf("\n================ GANTT CHART ================\n");

    int last_mode = -1;
    int last_innings = -1;

    for (int i = 0; i < gantt_count; i++)
    {
        // switch between SJF / FCFS
        if (gantt_chart[i].mode != last_mode)
        {
            printf("\n--- %s SIMULATION ---\n",
                   gantt_chart[i].mode == 0 ? "SJF" : "FCFS");
            last_mode = gantt_chart[i].mode;
            last_innings = -1;
        }

        // switch innings
        if (gantt_chart[i].innings != last_innings)
        {
            printf("\n[INNINGS %d]\n", gantt_chart[i].innings);
            last_innings = gantt_chart[i].innings;

            printf("%-10s %-15s %-15s\n", "Ball", "Bowler", "Batsman");
            printf("----------------------------------------------------\n");
        }

        // USE STORED NAMES (not pointers, not current arrays, not vibes)
        printf("%d.%d       %-15s %-15s\n",
               gantt_chart[i].over,
               gantt_chart[i].ball,
               gantt_chart[i].bowler_name,
               gantt_chart[i].batsman_name);
    }

    printf("\n");
}