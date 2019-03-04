//
// Created by vitek on 4.3.19.
//

#include "solution.h"
#include "constants.h"
#include "coords.h"

solution::solution(pole p, coords pos, int t1, int t2, int f, int c) : current_state(p), current_position(pos),
                                                                       type1_count(t1),
                                                                       type2_count(t2),
                                                                       free_count(f),
                                                                       cost(c) {};

void solution::recalculate_cost() {
    cost = type1_cost * type1_count + type2_cost * type2_count + free_count * free_cost;
}

bool solution::check_if_tile_fits(int length, coords pos, int direction) {

    if (direction == VERTICAL) {

        if (current_state.x_dim < pos.x + length) {
            return false;
        }

        for (int i = 0; i < length; i++) {
            if (current_state.map[pos.x + i][pos.y] != FREE_POS) {
                return false;
            }
        }

    } else if (direction == HORIZONTAL) {

        if (current_state.x_dim < pos.y + length) {
            return false;
        }

        for (int i = 0; i < length; i++) {
            if (current_state.map[pos.x][pos.y + i] != FREE_POS) {
                return false;
            }
        }
    }

    return true;
}

void solution::add_tile(int length, coords pos, int direction) {

    int id = current_state.y_dim * pos.x + pos.y;
    if (direction == VERTICAL) {

        for (int i = 0; i < length; i++) {
            current_state.map[pos.x + i][pos.y] = id;
        }

    } else if (direction == HORIZONTAL) {

        for (int i = 0; i < length; i++) {
            current_state.map[pos.x][pos.y + i] = id;
        }
    }

}

coords solution::next_position() {
    coords c;
    if (current_position.x >= current_state.x_dim - 1) {
        c.x = 0;
    } else {
        c.x = current_position.x + 1;
    }
    c.y = current_position.y + 1;
    return c;
}

void solution::print_map() {
    current_state.print();
}