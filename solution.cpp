//
// Created by vitek on 4.3.19.
//

#include "solution.h"
#include "constants.h"
#include "coords.h"

solution::solution(pole p, int t1, int t2, int f, int c1, int c2, int c3, int c) : current_state(p),
                                                                                   type1_count(t1),
                                                                                   type1_cost(c1),
                                                                                   type2_count(t2),
                                                                                   type2_cost(c2),
                                                                                   free_count(f),
                                                                                   free_cost(c3),
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

void solution::add_tile(int length, int type, coords pos, int direction) {

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

    if (type == 1) {
        type1_count++;
    } else {
        type2_count++;
    }
    free_count -= length;
}

coords solution::next_position(coords current) {
    coords c(-1, -1);

    if ((current.x >= current_state.x_dim - 1) && (current.y >= current_state.y_dim - 1)) {
        return c;
    }

    if (current.y >= current_state.y_dim - 1) {
        c.y = 0;
        c.x = current.x + 1;
    } else {
        c.x = current.x;
        c.y = current.y + 1;
    }
    return c;
}

void solution::print_map() {
    current_state.print();
    cout << endl;
}

coords solution::next_free_position(coords current) {
    coords c(-1, -1);
    int num_fields = current_state.x_dim * current_state.y_dim;
    int current_num = current_state.y_dim * current.x + current.y;

    for (int i = current_num + 1; i < num_fields; i++) {
        coords next = next_position(current);
        if (current_state.is_free(next.x, next.y)) {
            return next;
        }
        current = next;
    }
    return c;
}

void solution::remove_tile(int length, int type, coords pos, int direction) {
    if (direction == VERTICAL) {

        for (int i = 0; i < length; i++) {
            current_state.map[pos.x + i][pos.y] = FREE_POS;
        }

    } else if (direction == HORIZONTAL) {

        for (int i = 0; i < length; i++) {
            current_state.map[pos.x][pos.y + i] = FREE_POS;
        }
    }

    if (type == 1) {
        type1_count--;
    } else {
        type2_count--;
    }
    free_count += length;
}

solution::solution() {
    current_state = pole();
    current_position = coords(0,0);
    type1_count = 0;
    type1_cost = 0;
    type2_count = 0;
    type2_cost = 0;
    free_cost = 0;
    free_count = 0;
}

void solution::print_solution() {
    print_map();
    cout << "Cost: " << cost << endl;
    cout << "# T1: " << type1_count << ", # T2: " << type2_count << endl;
    cout << "# Free: " << free_count << endl;
    cout << endl;
}
