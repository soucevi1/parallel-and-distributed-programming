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

    int type1_count;
    int type1_cost;
    int type1_length;

    int type2_count;
    int type2_cost;
    int type2_length;

    int empty_cost;
    int empty_count;

    int shortest_tile_length;

    int deliberately_empty_count;

    int delib_empty_in_row;

    struct best{
        pole best_map;
        int best_cost;
        int type1_cnt;
        int type2_cnt;
        //void print_best();
    };

    best best_solution;

    solution();
    solution(pole map, int t1_count, int t2_count, int free_count, int t1_cost, int t2_cost, int free_cost,
                 int cost, int t1_len, int t2_len);

    void recalculate_cost();

    void add_tile(int length, int type, coords pos, int direction);

    void remove_tile(int length, int type, coords &pos, int direction);

    bool check_if_tile_fits(int length, coords &pos, int direction);

    coords next_position(coords current);

    coords next_free_position(coords current);

    void print_map();

    void print_solution();

    void print_best();

    void compare_best();

    bool could_be_better_than_best(coords &position);

    bool can_fit_tile_behind(coords &position);
    bool can_fit_tile_above(coords &position);

    int eval(int number);
};


#endif //MI_PDP_SOLUTION_H
