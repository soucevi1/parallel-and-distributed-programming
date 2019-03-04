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

    solution(pole p, coords pos, int t1, int t2, int f, int c);

    void recalculate_cost();

    void add_tile(int length, coords pos, int direction);

    bool check_if_tile_fits(int length, coords pos, int direction);

    coords next_position();

    void print_map();
};


#endif //MI_PDP_SOLUTION_H
