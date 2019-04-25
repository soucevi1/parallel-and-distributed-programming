//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_SOLVER_H
#define MI_PDP_SOLVER_H

#include <vector>
#include "coords.h"
#include "pole.h"
#include "solution.h"
#include "comm_info.h"
#include <queue>
#include <omp.h>

using namespace std;

class solver {
public:
    solver();
    solver(pole p, int i1, int i2, int c1, int c2, int cn);
    solver(deque<comm_info> received, int pc, int rank);
    solver(comm_info received, int pc, int rank);

    pole map;

    int type1_len;
    int type2_len;

    int type1_cost;
    int type2_cost;
    int free_cost;

    int total_processes;
    int my_rank;
    bool best_updated;

    struct initial_solution{
        solution starting_solution;
        coords position;
    };

    coords generating_position;
    solution generating_solution;
    solution best_solution;

    omp_lock_t best_lock;

    void generate_initial_solutions(int required_levels);
    void generate_solutions_for_slaves(int required_levels);

    deque<initial_solution> initial_solutions;

    void solve();

    void find_cover(solution &s, coords &position, int tile_length, int tile_orientation, int tile_type);

    void initiate_search(solution & s, coords initial_position);

    void compare_with_best(solution &sol, bool should_tell_others);

    void check_best_message();

    void send_best_message();

};


#endif //MI_PDP_SOLVER_H