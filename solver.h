//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_SOLVER_H
#define MI_PDP_SOLVER_H

#include <vector>
#include "coords.h"
#include "pole.h"
#include "solution.h"
#include <queue>

using namespace std;

class solver {
public:
    solver(pole p, int i1, int i2, int c1, int c2, int cn);

    pole map;

    int type1_len;
    int type2_len;

    int type1_cost;
    int type2_cost;
    int free_cost;

    void print();

    struct initial_solution{
        solution starting_solution;
        coords position;
    };

    solution best_solution;

    void generate_initial_solutions(int required_levels);

    queue<initial_solution> initial_solutions;

    void solve();

    void find_cover(solution s, const coords &position, int tile_length, int tile_orientation, int tile_type);

    void initiate_search(solution s, coords initial_position);

    void compare_with_best(solution &sol);

};


#endif //MI_PDP_SOLVER_H