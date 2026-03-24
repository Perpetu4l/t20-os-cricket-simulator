#include "../include/simulator.h"
#include <stdio.h>

GanttCell gantt_chart[MAX_BALL_EVENTS];
int gantt_count = 0;
int mode;
int innings;

void record_gantt(int bowler,int batsman,int over,int ball)
{
    if(gantt_count >= MAX_BALL_EVENTS)
        return;

    gantt_chart[gantt_count].bowler_id = bowler;
    gantt_chart[gantt_count].batsman_id = batsman;
    gantt_chart[gantt_count].over = over;
    gantt_chart[gantt_count].ball = ball;

    gantt_chart[gantt_count].mode = scheduling_type;
    gantt_chart[gantt_count].innings = innings;

    gantt_count++;
}

void print_gantt_chart()
{
    printf("\n================ GANTT CHART ================\n");

    int last_mode = -1;
    int last_innings = -1;

    for(int i=0;i<gantt_count;i++)
    {
        if(gantt_chart[i].mode != last_mode){
            printf("\n--- %s SIMULATION ---\n",
                gantt_chart[i].mode == 0 ? "SJF" : "FCFS");
            last_mode = gantt_chart[i].mode;
            last_innings = -1;
        }

        if(gantt_chart[i].innings != last_innings){
            printf("\n[INNINGS %d]\n", gantt_chart[i].innings);
            last_innings = gantt_chart[i].innings;

            printf("%-10s %-10s %-10s\n","Ball","Bowler","Batsman");
            printf("------------------------------------------\n");
        }

        printf("%d.%d       %-10s %-10s\n",
            gantt_chart[i].over,
            gantt_chart[i].ball,
            bowlers[gantt_chart[i].bowler_id].name,
            batsmen[gantt_chart[i].batsman_id].name
        );
    }

    printf("\n");
}