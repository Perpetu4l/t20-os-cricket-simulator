typedef struct {
    int runs;
    int wickets;
    int overs;
    int balls;
} Scoreboard;

typedef struct {
    int id;
    char name[50];
    int runs;
    int balls_faced;
} Batsman;

typedef struct {
    int id;
    int balls_bowled;
    int runs_given;
    int wickets;
} Bowler;

typedef struct {
    Scoreboard score;
    int ball_in_air;
    int match_running;
} MatchState;

extern MatchState match;
extern pthread_mutex_t score_mutex;
extern pthread_mutex_t pitch_mutex;