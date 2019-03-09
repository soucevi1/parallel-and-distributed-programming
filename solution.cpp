//
// Created by vitek on 4.3.19.
//

#include "solution.h"
#include "constants.h"
#include "coords.h"
#include <limits.h>

solution::solution(pole map, int t1_count, int t2_count, int free_count, int t1_cost, int t2_cost, int free_cost,
                   int cost, int t1_len, int t2_len) : current_state(map),
                                                       type1_count(t1_count),
                                                       type1_cost(t1_cost),
                                                       type2_count(t2_count),
                                                       type2_cost(t2_cost),
                                                       free_count(free_count),
                                                       free_cost(free_cost),
                                                       cost(cost),
                                                       type1_length(t1_len),
                                                       type2_length(t2_len) {
    best_solution.best_map = current_state;
    best_solution.best_cost = INT_MIN;
    best_solution.type1_cnt = 0;
    best_solution.type2_cnt = 0;

    best_cost_per_field = 0;
    double type1_rel = (double) type1_cost / type1_length;
    double type2_rel = (double) type2_cost / type2_length;

    if (type1_rel > type2_rel) {
        best_cost_per_field = type1_rel;
    } else {
        best_cost_per_field = type2_rel;
    }

};

void solution::recalculate_cost() {
    cost = type1_cost * type1_count + type2_cost * type2_count + free_count * free_cost;
}

bool solution::check_if_tile_fits(int length, coords &pos, int direction) {

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

void solution::remove_tile(int length, int type, coords &pos, int direction) {
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
    current_position = coords(0, 0);
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

void solution::compare_best() {
    if (cost > best_solution.best_cost) {
        best_solution.best_cost = cost;
        best_solution.best_map = current_state;
        best_solution.type1_cnt = type1_count;
        best_solution.type2_cnt = type2_count;

        best_solution.print_best();
        cout << endl;
    }
}

bool solution::could_be_better_than_best(coords &position) {

    int fields_to_cover = get_following_uncovered_fields(position);

    double hypothetical_max_increase = fields_to_cover * best_cost_per_field;

    double hypothetical_max_cost = hypothetical_max_increase + cost - fields_to_cover*free_cost;

    return hypothetical_max_cost > (double)best_solution.best_cost;
}

int solution::get_following_uncovered_fields(coords &position) {
    int counter = 0;
    for (int i = position.y; i < current_state.y_dim; i++) {
        if (current_state.map[position.x][i] == FREE_POS) {
            counter++;
        }
    }

    for (int i = 0; i < current_state.x_dim-position.x-1; i++) {
        for (int j = 0; j < current_state.y_dim; j++) {
            if (current_state.map[1+i+position.x][j] == FREE_POS) {
                counter++;
            }
        }
    }
    //cout << "pos: " << position.x << "," << position.y << " [" << current_state.x_dim << "," << current_state.y_dim << "]"  << " unc: " << counter << endl;
    return counter;
}

int solution::eval(int number) {
    // vraci maximalni cenu pro "number" nevyresenych policek
    // returns the maximal price for the "number" of unsolved squares
    int i, x;
    int max = type2_cost * (number / type2_length);
    int zb = number % type2_length;
    max += type1_cost * (zb / type1_length);
    zb = zb % type1_length;
    max += zb * free_cost;
    for (i = 0; i < (number / type2_length); i++) {
        x = type2_cost * i;
        zb = number - i * type2_length;
        x += type1_cost * (zb / type1_length);
        zb = zb % type1_length;
        x += zb * free_cost;
        if (x > max) max = x;
    }
    return max;
}

bool solution::can_fit_tile_behind(coords &position) {
    int shortest_tile_length;
    shortest_tile_length = type1_length > type2_length ? type2_length : type1_length;

    if(position.y - shortest_tile_length < 0){
        return false;
    }

    for(int i=position.y; i > position.y-shortest_tile_length; i--){
        if(current_state.map[position.x][i] != FREE_POS){
            return false;
        }
    }
    return true;
}

bool solution::can_fit_tile_above(coords &position) {
    int shortest_tile_length;
    shortest_tile_length = type1_length > type2_length ? type2_length : type1_length;

    if(position.x - shortest_tile_length < 0){
        return false;
    }

    for(int i=position.x; i > position.x-shortest_tile_length; i--){
        if(current_state.map[i][position.y] != FREE_POS){
            return false;
        }
    }
    return true;
}

void solution::best::print_best() {
    best_map.print();
    cout << "Cost: " << best_cost << endl;
    cout << "T1: " << type1_cnt << " T2: " << type2_cnt << endl;
}
