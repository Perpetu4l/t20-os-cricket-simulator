#include "../include/simulator.h"

pthread_t bowler_thread_id;
pthread_t batsman_threads[2];
pthread_t fielder_threads[MAX_FIELDERS];

void* bowler_thread(void* arg) {

    while(match.match_running) {

        pthread_mutex_lock(&pitch_mutex);

        pitch_ball = generate_ball_event();

        printf("Bowler delivered ball result %d\n",pitch_ball);

        pthread_mutex_unlock(&pitch_mutex);

        sleep(1);
    }

    return NULL;
}

void* batsman_thread(void* arg) {

    int id = *(int*)arg;

    while(match.match_running) {

        pthread_mutex_lock(&pitch_mutex);

        if(pitch_ball != -2) {

            int result = pitch_ball;

            update_score(result);

            log_ball(
                match.score.overs + 1,
                match.score.balls + 1,
                result
            );

            pitch_ball = -2;

            match.score.balls++;

            if(match.score.balls == 6) {
                match.score.balls = 0;
                match.score.overs++;
            }

        }

        pthread_mutex_unlock(&pitch_mutex);

        sleep(1);
    }

    return NULL;
}

void* fielder_thread(void* arg) {

    int id = *(int*)arg;

    while(match.match_running) {

        printf("Fielder %d ready\n", id);

        sleep(3);
    }

    return NULL;
}

void create_players() {

    pthread_create(&bowler_thread_id, NULL, bowler_thread, NULL);

    int batsman_ids[2] = {1,2};

    for(int i=0;i<2;i++) {
        pthread_create(&batsman_threads[i], NULL, batsman_thread, &batsman_ids[i]);
    }

    int fielder_ids[MAX_FIELDERS];

    for(int i=0;i<MAX_FIELDERS;i++) {
        fielder_ids[i] = i+1;
        pthread_create(&fielder_threads[i], NULL, fielder_thread, &fielder_ids[i]);
    }
}