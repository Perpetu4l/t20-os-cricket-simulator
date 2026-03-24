#include "../include/simulator.h"

pthread_t batsman_threads[2];
pthread_t fielder_threads[MAX_FIELDERS];

void* bowler_thread(void* arg){

    int id = *(int*)arg;

    while(match.match_running){

        // wait for innings
        pthread_mutex_lock(&start_mutex);
        while(!innings_started){
            pthread_cond_wait(&start_cond, &start_mutex);
        }
        pthread_mutex_unlock(&start_mutex);

        pthread_mutex_lock(&pitch_mutex);

        if(id != current_bowler){
            pthread_mutex_unlock(&pitch_mutex);
            usleep(1000);
            continue;
        }

        // 🔥 JUST DELIVER BALL (NO RESULT HERE)
        if(pitch_ball == -2 && match.run_in_progress == 0){
            pitch_ball = 1;   // ball delivered
        }

        pthread_mutex_unlock(&pitch_mutex);

        sleep(1);  // pacing
    }

    return NULL;
}

void* batsman_thread(void* arg){

    int id = *(int*)arg;

    sem_wait(&crease_sem);

    while(match.match_running){

        pthread_mutex_lock(&start_mutex);
        while(!innings_started){
            pthread_cond_wait(&start_cond, &start_mutex);
        }
        pthread_mutex_unlock(&start_mutex);

        pthread_mutex_lock(&pitch_mutex);

        if(match.score.overs >= MAX_OVERS || (innings == 2 && match.score.runs >= target_score)){
            pthread_mutex_unlock(&pitch_mutex);

            // FORCE THREAD TO WAIT FOR NEXT INNINGS
            pthread_mutex_lock(&start_mutex);
            while(innings_started){   // wait until main sets it to 0
                pthread_cond_wait(&start_cond, &start_mutex);
            }
            pthread_mutex_unlock(&start_mutex);

            continue;
        }

        if(pitch_ball == -2){
            pthread_mutex_unlock(&pitch_mutex);
            usleep(1000);
            continue;
        }

        /* NON-STRIKER */
        if(id != 0){
            pthread_mutex_unlock(&pitch_mutex);

            pthread_mutex_lock(&run_mutex);
            while(run_ready == 0)
                pthread_cond_wait(&run_cond, &run_mutex);
            run_ready = 0;
            pthread_mutex_unlock(&run_mutex);

            int waiting = attempt_run(id);
            if(waiting) usleep(1000);

            continue;
        }

        /* STRIKER */

        int deadlock_happened = 0;
        int wicket_happened = 0;
        int next_batsman = -1;

        int batsman_id = match.striker;
        int result = generate_ball_event(&batsmen[batsman_id]);
        //  GLOBAL TIME STEP (ONE BALL PROCESSED)
        global_time++;
        pitch_ball = -2;

        Batsman *bat = &batsmen[batsman_id];
        // 🎯 FIRST TIME THIS BATSMAN PLAYS
        if(!bat->has_started){
            bat->start_time = global_time;
            bat->wait_time = bat->start_time - bat->arrival_time;
            bat->has_started = 1;
        }
        Bowler *b = &bowlers[current_bowler];

        int runs_attempted = result;
        int runs_completed = result;
        int victim = -1;

        if(result != 7){
            b->balls_bowled++;
            bat->balls_faced++;
        }

        /* RUN LOGIC */

        if(result == 1 || result == 2 || result == 3){

            match.run_in_progress = 1;

            pthread_mutex_lock(&run_mutex);
            run_ready = 1;
            pthread_cond_signal(&run_cond);
            pthread_mutex_unlock(&run_mutex);

            pthread_mutex_unlock(&pitch_mutex);

            int waiting = attempt_run(id);

            if(waiting && detect_deadlock()){

                deadlock_happened = 1;

                runs_completed = runs_attempted - 1;
                if(runs_completed < 0) runs_completed = 0;

                /* decide victim CORRECTLY */
                if(runs_completed % 2 == 0)
                    victim = match.striker;
                else
                    victim = match.non_striker;

                result = -1;
            }

            match.run_in_progress = 0;
            pthread_mutex_lock(&pitch_mutex);
        }

        /* APPLY RUNS */

        if(result == 7){   // WIDE
            match.score.extras += 1;
            b->runs_given += 1;
            update_score(1);
        }
        else if(deadlock_happened){
            bat->runs += runs_completed;
            b->runs_given += runs_completed;
            update_score(runs_completed);
        }
        else if(result != -1){
            bat->runs += runs_attempted;
            b->runs_given += runs_attempted;
            update_score(runs_attempted);

            if(result == 4){
                bat->fours++;
            }
            if(result == 6){
                bat->sixes++;
            }
        }

     
        /* STRIKE */

        int runs_for_strike = deadlock_happened ? runs_completed : runs_attempted;

        if(result != 7 && result != -1){  
            // normal case
            if(runs_for_strike % 2 == 1){
                swap_strike();
            }
        }

        /* WICKET */

        if(result == -1){

            int out_player = deadlock_happened ? victim : batsman_id;

            // 🔥 FIRST: apply strike based on completed runs
            if(deadlock_happened){
                if(runs_completed % 2 == 1){
                    swap_strike();
                }
            }

            batsmen[out_player].is_out = 1;
            b->wickets++;
            update_score(-1);
            

            if(scheduling_type == 0)
                next_batsman = sjf_scheduler();
            else
                next_batsman = fcfs_scheduler();
            
            /* 🔥 SAFETY CHECK */
            if(next_batsman < 0 || next_batsman >= MAX_BATSMEN){
                pthread_mutex_unlock(&pitch_mutex);
                continue;
            }

            /* 🔥 SET ARRIVAL TIME HERE */
            if(!batsmen[next_batsman].has_started){
    batsmen[next_batsman].arrival_time = global_time;
}
            wicket_happened = 1;

            if(out_player == match.striker)
                match.striker = next_batsman;
            else
                match.non_striker = next_batsman;
        }

        /* LOGGING */

        int display_over = match.score.overs;
        int display_ball;

        if(result != 7){

            match.score.balls++;

            if(match.score.balls == 6){

                display_over = match.score.overs + 1;
                display_ball = 0;

                log_ball(display_over, display_ball, result, batsman_id);
                record_gantt(current_bowler, batsman_id,
                             display_over, display_ball);
                
                             if(match.score.wickets >= 10){
                        pthread_mutex_unlock(&pitch_mutex);
                        break;
                    }


                if(deadlock_happened){
                    resolve_deadlock();

                    pthread_mutex_lock(&print_mutex);

                    printf("  [UMPIRE] RUN OUT! %s is dismissed\n",
                        batsmen[victim].name);

                    pthread_mutex_unlock(&print_mutex);
                }


                if(wicket_happened && match.score.wickets < 10){
                    pthread_mutex_lock(&print_mutex);
                    printf("  [%s SCHED] Next batsman: %s (pos=%d)\n",
    (scheduling_type == 0 ? "SJF" : "FCFS"),
    batsmen[next_batsman].name, next_batsman+1);
                    pthread_mutex_unlock(&print_mutex);
                }

                match.score.balls = 0;
                match.score.overs++;

                swap_strike();
                priority_scheduler();
                round_robin_scheduler();

            } else {

                display_ball = match.score.balls;

                log_ball(display_over, display_ball, result, batsman_id);
                record_gantt(current_bowler, batsman_id,
                             display_over, display_ball);
                
                             if(match.score.wickets >= 10){
    pthread_mutex_unlock(&pitch_mutex);
    break;
}

                if(deadlock_happened){
                    resolve_deadlock();

                    pthread_mutex_lock(&print_mutex);

                    printf("  [UMPIRE] RUN OUT! %s is dismissed\n",
                        batsmen[victim].name);

                    pthread_mutex_unlock(&print_mutex);
                }



                if(wicket_happened && match.score.wickets < 10){
                    pthread_mutex_lock(&print_mutex);
                    printf("  [%s SCHED] Next batsman: %s (pos=%d)\n",
    (scheduling_type == 0 ? "SJF" : "FCFS"),
    batsmen[next_batsman].name, next_batsman+1);
                    pthread_mutex_unlock(&print_mutex);
                }
            }

        } else {

            display_ball = match.score.balls;

            log_ball(display_over, display_ball, result, batsman_id);
            record_gantt(current_bowler, batsman_id,
                         display_over, display_ball);

            if(match.score.wickets >= 10){
    pthread_mutex_unlock(&pitch_mutex);
    break;
}

            if(deadlock_happened){
                resolve_deadlock();
            }
        }

        pthread_mutex_unlock(&pitch_mutex);
    }

    sem_post(&crease_sem);
    return NULL;
}


/* FIELDER */

void* fielder_thread(void* arg){

    int id = *(int*)arg;

    while(match.match_running){

        pthread_mutex_lock(&fielder_mutex);

        while(match.ball_in_air == 0){
            pthread_cond_wait(&ball_hit_cond, &fielder_mutex);
        }

        pthread_mutex_unlock(&fielder_mutex);

        sleep(rand() % 2 + 1);
    }

    return NULL;
}

/* INIT */

void create_players(){

    static int bowler_ids[MAX_BOWLERS];

    for(int i=0;i<MAX_BOWLERS;i++){
        bowler_ids[i] = i;
        pthread_create(&bowler_threads[i], NULL, bowler_thread, &bowler_ids[i]);
    }

    static int batsman_ids[2] = {0,1};

    for(int i=0;i<2;i++){
        pthread_create(&batsman_threads[i], NULL, batsman_thread, &batsman_ids[i]);
    }

    static int fielder_ids[MAX_FIELDERS];

    for(int i=0;i<MAX_FIELDERS;i++){
        fielder_ids[i] = i+1;
        pthread_create(&fielder_threads[i], NULL, fielder_thread, &fielder_ids[i]);
    }
}