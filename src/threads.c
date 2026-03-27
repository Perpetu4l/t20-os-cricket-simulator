#include "../include/simulator.h"
#include <errno.h>

pthread_t batsman_threads[2];
pthread_t fielder_threads[MAX_FIELDERS];

/*
   HOW THE RACE WORKS (read this before touching timing values)
   ─────────────────────────────────────────────────────────────
   crease_mutex is the "crease token".  It is grabbed by whoever
   arrives first and HELD until the other side has had a chance
   to trylock and see it is taken.

   BATSMAN wins  → batsman grabs crease_mutex, holds it while
                   fielder_done is waited on, then releases.
                   Fielder's trylock fails → batsman safe.

   FIELDER wins  → fielder grabs crease_mutex, sets runout flag,
                   sets fielder_done, KEEPS holding mutex.
                   Batsman's trylock fails → reads flag → run out.
                   Fielder releases mutex after batsman reads flag
                   (signalled via fielder_mutex cond or short sleep).

   chosen_fielder     : set by striker before broadcast — exactly 1 fielder acts.
   fielder_runout_happened : set by fielder on win.
   winning_fielder_id : for display.
   fielder_done       : set by fielder once race is resolved (win or loss).
   All protected by fielder_mutex EXCEPT crease_mutex itself.
*/
static volatile int chosen_fielder          = -1;
static volatile int fielder_runout_happened =  0;
static volatile int winning_fielder_id      = -1;
static volatile int fielder_done            =  0;

/* ═══════════════════════════════════════════
   BOWLER THREAD
═══════════════════════════════════════════ */
void *bowler_thread(void *arg)
{
    int id = *(int *)arg;
    while (match.match_running)
    {
        pthread_mutex_lock(&start_mutex);
        while (!innings_started)
            pthread_cond_wait(&start_cond, &start_mutex);
        pthread_mutex_unlock(&start_mutex);

        pthread_mutex_lock(&pitch_mutex);
        if (id != current_bowler) {
            pthread_mutex_unlock(&pitch_mutex);
            usleep(100000);
            continue;
        }
        if (pitch_ball == -2 && match.run_in_progress == 0)
            pitch_ball = 1;
        pthread_mutex_unlock(&pitch_mutex);
        // sleep(1);
        usleep(100000);
    }
    return NULL;
}

/* ═══════════════════════════════════════════
   BATSMAN THREAD
═══════════════════════════════════════════ */
void *batsman_thread(void *arg)
{
    int id = *(int *)arg;
    sem_wait(&crease_sem);

    while (match.match_running)
    {
        pthread_mutex_lock(&start_mutex);
        while (!innings_started)
            pthread_cond_wait(&start_cond, &start_mutex);
        pthread_mutex_unlock(&start_mutex);

        pthread_mutex_lock(&pitch_mutex);

        if ((match.score.overs >= MAX_OVERS) ||
            (match.score.wickets >= 10) ||
            (innings == 2 && match.score.runs >= target_score))
        {
            pthread_mutex_unlock(&pitch_mutex);
            pthread_mutex_lock(&start_mutex);
            while (innings_started)
                pthread_cond_wait(&start_cond, &start_mutex);
            pthread_mutex_unlock(&start_mutex);
            continue;
        }

        if (pitch_ball == -2) {
            pthread_mutex_unlock(&pitch_mutex);
            usleep(1000);
            continue;
        }

        /* ══ NON-STRIKER ══ */
        if (id != 0) {
            pthread_mutex_unlock(&pitch_mutex);
            pthread_mutex_lock(&run_mutex);
            while (run_ready == 0)
                pthread_cond_wait(&run_cond, &run_mutex);
            run_ready = 0;
            pthread_mutex_unlock(&run_mutex);
            int waiting = attempt_run(id);
            if (waiting) usleep(1000);
            continue;
        }

        /* ══ STRIKER ══ */
        int deadlock_happened = 0;
        int fielder_runout    = 0;
        int wicket_happened   = 0;
        int next_batsman_idx  = -1;
        int victim            = -1;
        int saved_fielder_id  = -1;

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
        if (!bat->has_started) {
            bat->start_time  = global_time;
            bat->wait_time   = bat->start_time - bat->arrival_time;
            bat->has_started = 1;
        }

        Bowler *b = &bowlers[current_bowler];
        int runs_attempted = result;
        int runs_completed = result;

        if (is_legal) {
            // b->balls_bowled++;
            bat->balls_faced++;
        }

        /* ══ RUN LOGIC — only 1, 2, 3 ══ */
        if (result == 1 || result == 2 || result == 3)
        {       
            match.run_in_progress = 1;

            /* arm the race — pick ONE fielder, then broadcast */
            pthread_mutex_lock(&fielder_mutex);
            chosen_fielder          = rand() % MAX_FIELDERS + 1;
            fielder_runout_happened = 0;
            winning_fielder_id      = -1;
            fielder_done            = 0;
            match.ball_in_air       = 1;
            pthread_cond_broadcast(&ball_hit_cond);
            pthread_mutex_unlock(&fielder_mutex);

            /* BATSMAN SPRINT: 70-130ms */
            // NEW (balanced)
            usleep(rand() % 10000 + 10000);   // 30–70 ms

            /* ── RACE: try to grab crease before fielder ── */
            if (pthread_mutex_trylock(&crease_mutex) == 0)
            {
                /* BATSMAN WINS — hold mutex so fielder's trylock fails */
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
                chosen_fielder    = -1;
                pthread_mutex_unlock(&fielder_mutex);

                pthread_mutex_lock(&run_mutex);
                run_ready = 1;
                pthread_cond_signal(&run_cond);
                pthread_mutex_unlock(&run_mutex);

                pthread_mutex_unlock(&pitch_mutex);

                int waiting = attempt_run(id);
                if (waiting && detect_deadlock()) {
                    deadlock_happened = 1;
                    runs_completed    = runs_attempted - 1;
                    if (runs_completed < 0) runs_completed = 0;
                    victim = (runs_completed % 2 == 0)
                             ? match.striker : match.non_striker;
                    result = -1;
                }

                match.run_in_progress = 0;
                pthread_mutex_lock(&pitch_mutex);
            }
            else
            {
                /* BATSMAN LOST — fielder holds crease_mutex.
                   Wait for fielder to set fielder_done.
                   Must release pitch_mutex here — same as BATSMAN WINS branch —
                   otherwise the pthread_mutex_lock(&pitch_mutex) at the end of
                   this block double-locks it and deadlocks the striker forever. */
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
                chosen_fielder    = -1;
                pthread_mutex_unlock(&fielder_mutex);

                if (fielder_runout) {
                    victim         = match.striker;
                    result         = -1;
                    runs_completed = 0;
                }

                /* FIX: signal non-striker ONLY if there's actually a run to complete.
                   When fielder wins with a runout, no run occurred so don't wake
                   the non-striker into attempt_run — there's nothing to run. */
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

        /* ══ APPLY RUNS ══ */
        if (result == 7) {
            match.score.extras += 1;
            b->runs_given      += 1;
            update_score(1);
        } else if (deadlock_happened) {
            bat->runs     += runs_completed;
            b->runs_given += runs_completed;
            update_score(runs_completed);
        } else if (fielder_runout) {
            /* no runs */
        } else if(result==8){
            match.score.extras += 1;
            b->runs_given      += 1;
            free_hit=1;
            update_score(1);
        }
        else if (result != -1) {

            bat->runs     += runs_attempted;
            b->runs_given += runs_attempted;
            update_score(runs_attempted);
            if (result == 4) bat->fours++;
            if (result == 6) bat->sixes++;
            // free_hit=0;
        }

        int striker_before = match.striker;
        int non_striker_before = match.non_striker;

        /* ══ STRIKE ROTATION ══ */
        int runs_for_strike = deadlock_happened ? runs_completed : runs_attempted;
        if (is_legal && result != -1)
            if (runs_for_strike % 2 == 1)
                swap_strike();

        /* ══ WICKET HANDLING ══ */
        if (result == -1) {
            
            int out_player = (deadlock_happened || fielder_runout)
                             ? victim : batsman_id;

            if (deadlock_happened && runs_completed % 2 == 1)
                swap_strike();

            
            if (!deadlock_happened && !fielder_runout && !free_hit)
            b->wickets++;
            
            if (deadlock_happened||fielder_runout||!free_hit) 
            {
                // free_hit=0;
                update_score(-1);
                batsmen[out_player].is_out = 1;
                next_batsman_idx = get_next_batsman();

                if (next_batsman_idx >= 0 && next_batsman_idx < MAX_BATSMEN) {
                    batsmen[next_batsman_idx].arrival_time = global_time;
                    wicket_happened = 1;
                    if (out_player == match.striker)
                        match.striker = next_batsman_idx;
                    else
                        match.non_striker = next_batsman_idx;
                } else {
                    next_batsman_idx = -1;
                    wicket_happened  = 0;
                }
            }
            // if(is_legal){
            //     free_hit=0;
            // }
        }   

        /* ══ LOGGING ══ */

        int dismissal_type = OUT_NONE;

        if (deadlock_happened)
            dismissal_type = OUT_DEADLOCK;
        else if (fielder_runout)
            dismissal_type = OUT_RUNOUT;
        else if (result == -1)
            dismissal_type = OUT_BOWLED;
        

        int display_over = match.score.overs;
        int display_ball;

        if (is_legal) {
            int end_of_over = (match.score.balls == 6);
            display_over = end_of_over ? match.score.overs + 1 : match.score.overs;
            display_ball = end_of_over ? 0 : match.score.balls;

            int was_free_hit = free_hit;

            log_ball(display_over, display_ball, result,
                    match.striker, match.non_striker,
                    striker_before, non_striker_before,
                    dismissal_type, saved_fielder_id,
                    free_hit);

            record_gantt(current_bowler, batsman_id, display_over, display_ball);

            // ✅ RESET FREE HIT ONLY AFTER LOGGING
            if (is_legal)
                free_hit = 0;

            // 🔥 CONTEXT LINE (OS + CRICKET HYBRID, CLEAN)
            if (deadlock_happened) {
                resolve_deadlock();
                pthread_mutex_lock(&print_mutex);
                printf("         + Deadlock resolved : Run-out due to circular wait\n");
                pthread_mutex_unlock(&print_mutex);
            }
            else if (fielder_runout) {
                pthread_mutex_lock(&print_mutex);
                printf("         + Direct hit by fielder %d\n", saved_fielder_id);
                pthread_mutex_unlock(&print_mutex);
            }

            // 🔥 NEW BATSMAN (CLEANER)
            if (wicket_happened && match.score.wickets < 10) {
                pthread_mutex_lock(&print_mutex);
                printf("         + New batsman: %s (pos %d)\n\n",
                    batsmen[next_batsman_idx].name,
                    next_batsman_idx + 1);
                pthread_mutex_unlock(&print_mutex);
            }

            // 🔥 OVER END CLEANUP
            if (end_of_over) {
                match.score.balls = 0;
                match.score.overs++;
                swap_strike();

                priority_scheduler();
                round_robin_scheduler();

                pthread_mutex_lock(&run_mutex);
                pthread_cond_broadcast(&run_cond);
                pthread_mutex_unlock(&run_mutex);

                pthread_mutex_lock(&print_mutex);
                printf("\n  ── End of Over %d | Score: %d/%d ──\n\n",
                    match.score.overs,
                    match.score.runs,
                    match.score.wickets);
                pthread_mutex_unlock(&print_mutex);
            }
        }
        else {
            display_ball = match.score.balls;
            int was_free_hit = free_hit;
            log_ball(display_over, display_ball, result, match.striker, match.non_striker, striker_before, non_striker_before,dismissal_type,saved_fielder_id,was_free_hit);
            record_gantt(current_bowler, batsman_id, display_over, display_ball);
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

    sem_post(&crease_sem);
    return NULL;
}

/* ═══════════════════════════════════════════
   FIELDER THREAD
═══════════════════════════════════════════ */
void *fielder_thread(void *arg)
{
    int id = *(int *)arg;

    while (match.match_running)
    {
        pthread_mutex_lock(&fielder_mutex);
        while (match.ball_in_air == 0 && match.match_running)
            pthread_cond_wait(&ball_hit_cond, &fielder_mutex);

        if (!match.match_running) {
            pthread_mutex_unlock(&fielder_mutex);
            break;
        }

        if (id != chosen_fielder || match.ball_in_air == 0) {
            pthread_mutex_unlock(&fielder_mutex);
            continue;
        }
        pthread_mutex_unlock(&fielder_mutex);

        /* FIELDER SPRINT: 10–50ms (faster than batsman's 70–130ms = fielder usually wins) */
        // NEW
    usleep(rand() % 20000 + 19000);   // 40–120 ms

        if (pthread_mutex_trylock(&crease_mutex) == 0)
        {
            /* FIELDER WINS — set runout flag and done flag in one atomic section */
            pthread_mutex_lock(&fielder_mutex);
            if (match.ball_in_air == 1 && id == chosen_fielder) {
                fielder_runout_happened = 1;
                winning_fielder_id      = id;
            }
            fielder_done = 1;  /* must be set AFTER runout flag, inside same lock */
            pthread_mutex_unlock(&fielder_mutex);

            usleep(5000);  /* hold briefly so batsman's trylock definitely fails */
            pthread_mutex_unlock(&crease_mutex);
        }
        else
        {
            /* BATSMAN WON — just mark done */
            pthread_mutex_lock(&fielder_mutex);
            fielder_done = 1;
            pthread_mutex_unlock(&fielder_mutex);
        }
    }

    return NULL;
}

/* ═══════════════════════════════════════════
   CREATE ALL PLAYER THREADS
═══════════════════════════════════════════ */
void create_players()
{
    static int bowler_ids[MAX_BOWLERS];
    for (int i = 0; i < MAX_BOWLERS; i++) {
        bowler_ids[i] = i;
        pthread_create(&bowler_threads[i], NULL, bowler_thread, &bowler_ids[i]);
    }

    static int batsman_ids[2] = {0, 1};
    for (int i = 0; i < 2; i++)
        pthread_create(&batsman_threads[i], NULL, batsman_thread, &batsman_ids[i]);

    static int fielder_ids[MAX_FIELDERS];
    for (int i = 0; i < MAX_FIELDERS; i++) {
        fielder_ids[i] = i + 1;
        pthread_create(&fielder_threads[i], NULL, fielder_thread, &fielder_ids[i]);
    }
}