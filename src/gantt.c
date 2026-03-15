#include "../include/simulator.h"
#include <stdio.h>

GanttCell gantt_chart[MAX_BALL_EVENTS];
int gantt_count = 0;

void record_gantt(int bowler,int batsman,int over,int ball)
{
    if(gantt_count >= MAX_BALL_EVENTS)
        return;

    gantt_chart[gantt_count].bowler_id = bowler;
    gantt_chart[gantt_count].batsman_id = batsman;
    gantt_chart[gantt_count].over = over;
    gantt_chart[gantt_count].ball = ball;

    gantt_count++;
}

void print_gantt_chart()
{
    printf("\n================ GANTT CHART ================\n\n");

    printf("%-10s %-10s %-10s\n","Ball","Bowler","Batsman");
    printf("------------------------------------------\n");

    for(int i=0;i<gantt_count;i++)
    {
        printf("%d.%d       B%-2d        P%-2d\n",
            gantt_chart[i].over,
            gantt_chart[i].ball+1,
            gantt_chart[i].bowler_id,
            gantt_chart[i].batsman_id);
    }

    printf("\n");
}