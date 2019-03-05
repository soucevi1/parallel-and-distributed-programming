//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_SOLUTION_H
#define MI_PDP_SOLUTION_H

#include "pole.h"
#include "coords.h"

class solution {
public:

    pole current_state;

    int cost;

    coords current_position;

    int type1_count;
    int type1_cost;

    int type2_count;
    int type2_cost;

    int free_cost;
    int free_count;

    solution();
    solution(pole p, int t1, int t2, int f, int c1, int c2, int c3, int c);

    void recalculate_cost();

    void add_tile(int length, int type, coords pos, int direction);

    void remove_tile(int length, int type, coords pos, int direction);

    bool check_if_tile_fits(int length, coords pos, int direction);

    coords next_position(coords current);

    coords next_free_position(coords current);

    void print_map();

    void print_solution();
};


#endif //MI_PDP_SOLUTION_H
