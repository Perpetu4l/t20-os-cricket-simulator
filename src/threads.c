#include "../include/simulator.h"
#include <errno.h>
pthread_t batsman_threads[11];
pthread_t fielder_threads[11];
static int chosen_fielder  = -1;
static int fielder_runout_happened  =  0;
static int winning_fielder_id  =-1;
static int fielder_done =  0;
static int active_fielders_count=  0;

void *bowler_thread(void *arg)
{
    int id = *(int *)arg;

    while (match.match_running)
    {
        pthread_mutex_lock(&start_mutex);
        while (!innings_started) pthread_cond_wait(&start_cond, &start_mutex);
        pthread_mutex_unlock(&start_mutex);

        pthread_mutex_lock(&pitch_mutex);
        if (id != current_bowler) {
            pthread_mutex_unlock(&pitch_mutex);
            usleep(100000);
            continue;
        }
        if (pitch_ball == -2 && match.run_in_progress == 0) pitch_ball = 1;
        pthread_mutex_unlock(&pitch_mutex);
        usleep(100000);
    }
    return NULL;
}

void *batsman_thread(void *arg)
{
    int id = *(int *)arg;

    while (match.match_running)
    {
        
        pthread_mutex_lock(&start_mutex);
        while (!innings_started) pthread_cond_wait(&start_cond, &start_mutex);
        pthread_mutex_unlock(&start_mutex);

       
        pthread_mutex_lock(&batsman_mutex);
        while (id != match.striker && id != match.non_striker) pthread_cond_wait(&batsman_cond, &batsman_mutex);
        pthread_mutex_unlock(&batsman_mutex);

        
        pthread_mutex_lock(&crease_state_mutex);
        int already_in = batsmen[id].in_crease;
        pthread_mutex_unlock(&crease_state_mutex);

        if (!already_in) {
            sem_wait(&crease_sem);                 
            pthread_mutex_lock(&crease_state_mutex);
            batsmen[id].in_crease = 1;
            pthread_mutex_unlock(&crease_state_mutex);
        }

       
        pthread_mutex_lock(&pitch_mutex);

        
        if ((match.score.overs >= 20) ||
            (match.score.wickets >= 10) ||
            (innings == 2 && match.score.runs >= target_score))
        {
            pthread_mutex_unlock(&pitch_mutex);
            pthread_mutex_lock(&start_mutex);
            while (innings_started) pthread_cond_wait(&start_cond, &start_mutex);
            pthread_mutex_unlock(&start_mutex);
            continue;
        }

        if (pitch_ball == -2) {
            pthread_mutex_unlock(&pitch_mutex);
            usleep(1000);
            continue;
        }

        
        if (id == match.non_striker) {
            pthread_mutex_unlock(&pitch_mutex);

            pthread_mutex_lock(&run_mutex);
            while (run_ready == 0 && innings_started) pthread_cond_wait(&run_cond, &run_mutex);
            run_ready = 0;
            pthread_mutex_unlock(&run_mutex);
            if (!innings_started) continue;

            int waiting = attempt_run(id, 1);
            if (waiting) usleep(1000);
            continue;
        }

        int deadlock_happened = 0;
        int fielder_runout = 0;
        int wicket_happened = 0;
        int next_batsman_idx = -1;
        int victim = -1;
        int saved_fielder_id = -1;

        int batsman_id = match.striker;
        int result = generate_ball_event(&batsmen[batsman_id], &bowlers[current_bowler]);
        int original_result = result;
        int is_legal = (original_result != 7 && original_result != 8);

        if (is_legal) {
            match.score.balls++;
            bowlers[current_bowler].balls_bowled++;
        }

        global_time++;
        pitch_ball = -2;

        Batsman *bat = &batsmen[batsman_id];
       
        Bowler *b = &bowlers[current_bowler];
        int runs_attempted = result;
        int runs_completed = result;

        if (is_legal) bat->balls_faced++;
        if (result == 1 || result == 2 || result == 3)
        {
            match.run_in_progress = 1;

            pthread_mutex_lock(&fielder_mutex);
            chosen_fielder  = rand() % 11 + 1;
            fielder_runout_happened = 0;
            winning_fielder_id = -1;
            fielder_done = 0;
            active_fielders_count = 0;        
            match.ball_in_air = 1;
            pthread_cond_broadcast(&ball_hit_cond);
            pthread_mutex_unlock(&fielder_mutex);

            usleep(rand() % 20000 + 10000); 

            if (pthread_mutex_trylock(&crease_mutex) == 0)
            {
                int wc = 0;
                while (1) {
                    pthread_mutex_lock(&fielder_mutex);
                    int done = fielder_done;
                    pthread_mutex_unlock(&fielder_mutex);
                    if (done == 1) break;
                    usleep(3000);
                    if (++wc > 60) break;     
                }

                pthread_mutex_unlock(&crease_mutex);

                pthread_mutex_lock(&fielder_mutex);
                match.ball_in_air = 0;
                chosen_fielder = -1;
                active_fielders_count = 0;
                pthread_mutex_unlock(&fielder_mutex);
                pthread_mutex_lock(&run_mutex);
                run_ready = 1;
                pthread_cond_signal(&run_cond);
                pthread_mutex_unlock(&run_mutex);

                pthread_mutex_unlock(&pitch_mutex);

                int waiting = attempt_run(id, 0);
                if (waiting && detect_deadlock()) {
                    deadlock_happened = 1;
                    runs_completed    = runs_attempted - 1;
                    if (runs_completed < 0) runs_completed = 0;
                    victim = (runs_completed % 2 == 0) ? match.striker : match.non_striker;
                    result = -1;
                }

                match.run_in_progress = 0;
                pthread_mutex_lock(&pitch_mutex);
            }
            else
            {

                pthread_mutex_unlock(&pitch_mutex);

                int wc = 0;
                while (1) {
                    pthread_mutex_lock(&fielder_mutex);
                    int done = fielder_done;
                    pthread_mutex_unlock(&fielder_mutex);
                    if (done == 1) break;
                    usleep(3000);
                    if (++wc > 60) break;      
                }

                pthread_mutex_lock(&fielder_mutex);
                fielder_runout   = fielder_runout_happened;
                saved_fielder_id = winning_fielder_id;
                match.ball_in_air = 0;
                chosen_fielder = -1;
                active_fielders_count = 0;
                pthread_mutex_unlock(&fielder_mutex);

                if (fielder_runout) {
                    runs_completed    = runs_attempted - 1;
                    if (runs_completed < 0) runs_completed = 0;
                    victim = (runs_completed % 2 == 0) ? match.striker : match.non_striker;
                    result = -1;
                }

                if (!fielder_runout) {
                    pthread_mutex_lock(&run_mutex);
                    run_ready = 1;
                    pthread_cond_signal(&run_cond);
                    pthread_mutex_unlock(&run_mutex);
                }

                match.run_in_progress = 0;
                pthread_mutex_lock(&pitch_mutex);
            }
        }

        
        if (result == 7) {
            match.score.extras += 1;
            b->runs_given      += 1;
            update_score(1);
        } 
        else if (deadlock_happened) {
            bat->runs     += runs_completed;
            b->runs_given += runs_completed;
            update_score(runs_completed);
        } 
        else if (fielder_runout) {
            bat->runs     += runs_completed;
            b->runs_given += runs_completed;
            update_score(runs_completed);   
           
        } 
        else if (result == 8) {
            match.score.extras += 1;
            b->runs_given      += 1;
            free_hit = 1;
            update_score(1);
        } 
        else if (result != -1) {
            bat->runs     += runs_attempted;
            b->runs_given += runs_attempted;
            update_score(runs_attempted);
            if (result == 4) bat->fours++;
            if (result == 6) bat->sixes++;
        } 
        else if (result == -1) {
            if (victim == -1) victim = match.striker;
        }

        int striker_before = match.striker;
        int non_striker_before = match.non_striker;

        int runs_for_strike = ( deadlock_happened || fielder_runout ) ? runs_completed : runs_attempted;
        if (is_legal && result != -1)
            if (runs_for_strike % 2 == 1)
                swap_strike();

        pthread_mutex_lock(&batsman_mutex);
        pthread_cond_broadcast(&batsman_cond);
        pthread_mutex_unlock(&batsman_mutex);

        if (result == -1) {

            int out_player = (deadlock_happened || fielder_runout) ? victim : batsman_id;
            

            batsmen[out_player].turn_around_time = global_time;
            
            if (deadlock_happened && runs_completed % 2 == 1) swap_strike();
            if (!deadlock_happened && !fielder_runout && !free_hit) b->wickets++;

            if (deadlock_happened || fielder_runout || !free_hit)
            {
                update_score(-1);
                batsmen[out_player].is_out = 1;

                pthread_mutex_lock(&crease_state_mutex);
                batsmen[out_player].in_crease = 0;
                sem_post(&crease_sem);
                pthread_mutex_unlock(&crease_state_mutex);

                next_batsman_idx = get_next_batsman();

                if (next_batsman_idx >= 0 && next_batsman_idx < 11) {
                    wicket_happened = 1;
                    if (out_player == match.striker)
                        match.striker = next_batsman_idx;
                    else
                        match.non_striker = next_batsman_idx;
                } else {
                    next_batsman_idx = -1;
                    wicket_happened  = 0;
                }

                pthread_mutex_lock(&batsman_mutex);
                pthread_cond_broadcast(&batsman_cond);
                pthread_mutex_unlock(&batsman_mutex);
            }
        }
        int dismissal_type = 0;
        if (deadlock_happened)     dismissal_type = 3;
        else if (fielder_runout)   dismissal_type = 2;
        else if (result == -1)     dismissal_type = 1;

        int display_over = match.score.overs;
        int display_ball;

        if (is_legal) {
            int end_of_over = (match.score.balls == 6);
            display_over = end_of_over ? match.score.overs + 1 : match.score.overs;
            display_ball = end_of_over ? 0 : match.score.balls;

            log_ball(display_over, display_ball, result, match.striker, match.non_striker,striker_before, non_striker_before,dismissal_type, saved_fielder_id,free_hit, victim);
            record_gantt(current_bowler, batsman_id, display_over, display_ball, result, dismissal_type);

            if (is_legal) free_hit = 0;

            if (deadlock_happened) {
                resolve_deadlock();
                pthread_mutex_lock(&print_mutex);
                printf("         + Deadlock resolved : Run-out due to circular wait\n");
                pthread_mutex_unlock(&print_mutex);
            } 
           

            if (wicket_happened && match.score.wickets < 10) {
                pthread_mutex_lock(&print_mutex);
                printf("         + New batsman: %s (pos %d)\n\n",batsmen[next_batsman_idx].name,next_batsman_idx + 1);
                pthread_mutex_unlock(&print_mutex);
            }

            if (end_of_over) {
                match.score.balls = 0;
                match.score.overs++;
                swap_strike();

                pthread_mutex_lock(&batsman_mutex);
                pthread_cond_broadcast(&batsman_cond);
                pthread_mutex_unlock(&batsman_mutex);

                pthread_mutex_lock(&print_mutex);
                printf("\n  ── End of Over %d | Score: %d/%d ──\n\n", match.score.overs,match.score.runs,match.score.wickets);
                pthread_mutex_unlock(&print_mutex);

                if (match.score.overs < 20-1) round_robin_scheduler();
                else priority_scheduler();

               
                pthread_mutex_lock(&run_mutex);
                run_ready = 1;
                pthread_cond_broadcast(&run_cond);
                pthread_mutex_unlock(&run_mutex);

            }
        } 
        else {
            display_ball = match.score.balls;
            log_ball(display_over, display_ball, result,match.striker, match.non_striker,striker_before, non_striker_before,dismissal_type, saved_fielder_id,free_hit, victim);
            record_gantt(current_bowler, batsman_id, display_over, display_ball, result, dismissal_type);
    
        }

        if (match.score.wickets >= 10) {
            pthread_mutex_unlock(&pitch_mutex);
            pthread_mutex_lock(&start_mutex);
            while (innings_started)
                pthread_cond_wait(&start_cond, &start_mutex);
            pthread_mutex_unlock(&start_mutex);
            continue;
        }

        pthread_mutex_unlock(&pitch_mutex);
    }

    return NULL;
}

void *fielder_thread(void *arg)
{
    int id = *(int *)arg;

    while (match.match_running)
    {
        pthread_mutex_lock(&fielder_mutex);
        while (match.ball_in_air == 0 && match.match_running) pthread_cond_wait(&ball_hit_cond, &fielder_mutex);
        if (!match.match_running) {
            pthread_mutex_unlock(&fielder_mutex);
            break;
        }

        if (match.ball_in_air == 0) {
            pthread_mutex_unlock(&fielder_mutex);
            continue;
        }
        int activate = (rand() % 100) < 2;
        if (activate) active_fielders_count++;   

        pthread_mutex_unlock(&fielder_mutex);
        if (!activate) {
           
            pthread_mutex_lock(&fielder_mutex);
            active_fielders_count--;
            if (active_fielders_count <= 0) {
                active_fielders_count = 0;
                fielder_done = 1;       
            }
            pthread_mutex_unlock(&fielder_mutex);
            continue;    
        }             
            usleep(rand() % 60000 + 30000);    
        if (pthread_mutex_trylock(&crease_mutex) == 0)
        {
            
            pthread_mutex_lock(&fielder_mutex);

            if (match.ball_in_air == 1 && fielder_runout_happened == 0) {
                fielder_runout_happened = 1;
                winning_fielder_id      = id;
            }

            active_fielders_count--;
            if (active_fielders_count <= 0) {
                active_fielders_count = 0;
                fielder_done = 1;       
            }

            pthread_mutex_unlock(&fielder_mutex);

            usleep(5000);              
            pthread_mutex_unlock(&crease_mutex);
        }
        else
        {
           
            pthread_mutex_lock(&fielder_mutex);
            active_fielders_count--;
            if (active_fielders_count <= 0) {
                active_fielders_count = 0;
                fielder_done = 1;
            }
            pthread_mutex_unlock(&fielder_mutex);
        }
        
    }
    return NULL;
}

void create_players()
{
    static int bowler_ids[6];
    for (int i = 0; i < 6; i++) {
        bowler_ids[i] = i;
        pthread_create(&bowler_threads[i], NULL, bowler_thread, &bowler_ids[i]);
    }

    static int batsman_ids[11];
    for (int i = 0; i < 11; i++) {
        batsman_ids[i] = i;
        pthread_create(&batsman_threads[i], NULL, batsman_thread, &batsman_ids[i]);
    }

    static int fielder_ids[11];
    for (int i = 0; i < 11; i++) {
        fielder_ids[i] = i + 1;
        pthread_create(&fielder_threads[i], NULL, fielder_thread, &fielder_ids[i]);
    }
}