#include "../include/simulator.h"

// pthread_t bowler_thread_id;
pthread_t batsman_threads[2];
pthread_t fielder_threads[MAX_FIELDERS];

    
void* bowler_thread(void* arg){

    int id = *(int*)arg;

    while(match.match_running){

        if(id != current_bowler){
            sleep(1);
            continue;
        }

        pthread_mutex_lock(&pitch_mutex);

        if(pitch_ball == -2){

            pitch_ball = generate_ball_event();


            pthread_mutex_lock(&print_mutex);

            printf("Bowler %d delivered ball result %d\n",
                   id, pitch_ball);

            printf("Batsman %d played the ball\n",
                   match.striker);

            pthread_mutex_unlock(&print_mutex);
        }

        pthread_mutex_unlock(&pitch_mutex);

        sleep(1);
    }

    return NULL;
}

void* batsman_thread(void* arg) {

    int id = *(int*)arg;

    sem_wait(&crease_sem);

    while(match.match_running) {

        pthread_mutex_lock(&pitch_mutex);

        /* no ball available */
        if(pitch_ball == -2){
            pthread_mutex_unlock(&pitch_mutex);
            sleep(1);
            continue;
        }

        /* only ONE batsman thread processes the delivery */
        if(id != 1){
            pthread_mutex_unlock(&pitch_mutex);
            sleep(1);
            continue;
        }

        /* consume the ball atomically */
        int result = pitch_ball;
        pitch_ball = -2;

        int striker_id = match.striker;

        Batsman *bat = &batsmen[striker_id];
        Bowler *b = &bowlers[current_bowler];

        /* update stats */
        b->balls_bowled++;
        bat->balls_faced++;

        if(result == -1) {

            /* wicket */
            b->wickets++;
            bat->is_out = 1;

            update_score(result);

            int next = sjf_scheduler();

            pthread_mutex_lock(&print_mutex);
            printf("SJF Scheduler selected batsman %d\n", next);
            pthread_mutex_unlock(&print_mutex);

            match.striker = next;
        }
        else {

            if(result != 7) {
                b->runs_given += result;
                bat->runs += result;
            }

            update_score(result);

            /* strike change for odd runs */
            if(result == 1 || result == 3) {

                int temp = match.striker;
                match.striker = match.non_striker;
                match.non_striker = temp;
            }
        }

        log_ball(
            match.score.overs + 1,
            match.score.balls + 1,
            result
        );

        match.score.balls++;

        /* end of over */
        if(match.score.balls == 6) {

            match.score.balls = 0;
            match.score.overs++;

            /* strike rotates at over end */
            int temp = match.striker;
            match.striker = match.non_striker;
            match.non_striker = temp;

            priority_scheduler();// theek krna he ise

            if(match.score.overs < 19)
                round_robin_scheduler();
        }

        pthread_mutex_unlock(&pitch_mutex);

        /* wake fielders if ball hit */
        if(result != 7) {

            pthread_mutex_lock(&fielder_mutex);

            match.ball_in_air = 1;
            pthread_cond_broadcast(&ball_hit_cond);

            pthread_mutex_unlock(&fielder_mutex);

            sleep(1);

            pthread_mutex_lock(&fielder_mutex);
            match.ball_in_air = 0;
            pthread_mutex_unlock(&fielder_mutex);
        }
    }

    sem_post(&crease_sem);

    return NULL;
}

void* fielder_thread(void* arg) {

    int id = *(int*)arg;

    while(match.match_running) {

        pthread_mutex_lock(&fielder_mutex);

        while(match.ball_in_air == 0){
            pthread_cond_wait(&ball_hit_cond, &fielder_mutex);
        }

        pthread_mutex_unlock(&fielder_mutex);
        pthread_mutex_lock(&print_mutex);
        printf("Fielder %d chasing ball\n", id);
        pthread_mutex_unlock(&print_mutex);

        sleep(rand() % 2 + 1);
    }

    return NULL;
}


void create_players() {

    static int bowler_ids[MAX_BOWLERS];

    for(int i = 0; i < MAX_BOWLERS; i++){
        bowler_ids[i] = i;
        pthread_create(&bowler_threads[i], NULL, bowler_thread, &bowler_ids[i]);
    }

    static int batsman_ids[2] = {0,1};

    for(int i=0;i<2;i++) {
        pthread_create(&batsman_threads[i], NULL, batsman_thread, &batsman_ids[i]);
    }

    static int fielder_ids[MAX_FIELDERS];

    for(int i=0;i<MAX_FIELDERS;i++) {
        fielder_ids[i] = i+1;
        pthread_create(&fielder_threads[i], NULL, fielder_thread, &fielder_ids[i]);
    }
}