#include "simulator.h"

int main(){

    init_sync();

    create_players();

    start_match();

    end_match();

    return 0;
}