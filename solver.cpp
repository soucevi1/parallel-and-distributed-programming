//
// Created by vitek on 4.3.19.
//

#include <iostream>
#include "solver.h"
#include "pole.h"
#include "solution.h"
#include "constants.h"

using namespace std;

solver::solver(pole p, int i1, int i2, int c1, int c2, int cn) : map(p),
                                                                 type1_len(i1),
                                                                 type2_len(i2),
                                                                 type1_cost(c1),
                                                                 type2_cost(c2),
                                                                 free_cost(cn) {}


void solver::generate_initial_solutions(int required_levels) {
    queue<initial_solution> q;

    int free_count = map.x_dim * map.y_dim - map.forbidden_count;

    solution s(map, 0, 0, free_count, type1_cost, type2_cost, free_cost, 0, type1_len, type2_len);
    s.recalculate_cost();

    coords current_position(0, 0);
    if (!s.current_state.is_free(current_position.x, current_position.y)) {
        current_position = s.next_free_position(current_position);
    }

    initial_solution is;
    is.s = s;
    is.pos = current_position;

    q.push(is);

    for (int i = 1; i < required_levels; i++) {

        // For each tree level,
        // generate subproblems from the whole queue

        queue<initial_solution> q_2;

        int queue_size = (int) q.size();

        for (int j = 0; j < queue_size; j++) {
            initial_solution tmp = q.front();
            q.pop();

            current_position = tmp.pos;

            coords next_position;

            // TYPE 1 horizontal
            if (tmp.s.check_if_tile_fits(type1_len, current_position, HORIZONTAL)) {
                initial_solution tmp1 = tmp;
                tmp1.s.add_tile(type1_len, 1, current_position, HORIZONTAL);
                next_position = tmp1.s.next_free_position(tmp1.pos);
                if (next_position.x == -1) {
                    break;
                }
                tmp1.pos = next_position;
                tmp1.s.recalculate_cost();
                q_2.push(tmp1);
            }

            // TYPE 2 horizontal
            if (tmp.s.check_if_tile_fits(type2_len, current_position, HORIZONTAL)) {
                initial_solution tmp1 = tmp;
                tmp1.s.add_tile(type2_len, 2, current_position, HORIZONTAL);
                next_position = tmp1.s.next_free_position(tmp1.pos);
                if (next_position.x == -1) {
                    break;
                }
                tmp1.pos = next_position;
                tmp1.s.recalculate_cost();
                q_2.push(tmp1);
            }

            // TYPE 1 vertical
            if (tmp.s.check_if_tile_fits(type1_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.s.add_tile(type1_len, 1, current_position, VERTICAL);
                next_position = tmp1.s.next_free_position(tmp1.pos);
                if (next_position.x == -1) {
                    break;
                }
                tmp1.pos = next_position;
                tmp1.s.recalculate_cost();
                q_2.push(tmp1);
            }

            // TYPE 2 vertical
            if (tmp.s.check_if_tile_fits(type2_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.s.add_tile(type2_len, 2, current_position, VERTICAL);
                next_position = tmp1.s.next_free_position(tmp1.pos);
                if (next_position.x == -1) {
                    break;
                }
                tmp1.pos = next_position;
                tmp1.s.recalculate_cost();
                q_2.push(tmp1);
            }

            // LEAVE EMPTY
            next_position = tmp.s.next_free_position(tmp.pos);
            if (next_position.x == -1) {
                break;
            }
            tmp.pos = next_position;
            q_2.push(tmp);
        }

        q = q_2;
    }

    initial_solutions = q;
}

void solver::solve() {
    int required_number_of_levels = 1;

    generate_initial_solutions(required_number_of_levels);

    vector<solution> solutions;

    // Every worker gets one subproblem to solve
    for (int i = 0; i < initial_solutions.size(); i++) {
        initial_solution s = initial_solutions.front();
        initial_solutions.pop();

        initiate_search(s.s, s.pos);

        solutions.push_back(s.s);
    }

    solution best = solutions[0];
    for (int i = 1; i < solutions.size(); i++) {
        if (solutions[i].best_solution.best_cost > best.best_solution.best_cost) {
            best = solutions[i];
        }
    }
    cout << "The BEST solution is:" << endl;
    best.print_best();
}

void solver::find_cover(solution &s, coords &position, int tile_length, int tile_orientation, int tile_type) {

    bool tile_placed = false;
    bool dfc_increased = false;

    // If supposed to place a tile
    if (tile_orientation != LEAVE_EMPTY) {

        if (s.check_if_tile_fits(tile_length, position, tile_orientation)) {
            // If tile fits, add it and recalculate cost
            s.add_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
            s.compare_best();
            tile_placed = true;
            s.delib_empty_in_row = 0;

        } else {
            // Tile cannot be placed -- this branch ends
            return;
        }

    /*} else if (s.can_fit_tile_behind(position) || s.can_fit_tile_above(position)) {
        return;*/
    } else {
        s.deliberately_empty_count ++;
        s.delib_empty_in_row++;
        dfc_increased = true;
        if (s.can_fit_tile_behind(position) || s.can_fit_tile_above(position)) {
            s.deliberately_empty_count --;
            s.delib_empty_in_row--;
            return;
        }
    }

    coords next_position = s.next_free_position(position);

    // The search reached the end
    if (next_position.x == -1) {
        // If tile was placed previously, remove it
        if (tile_placed) {
            s.remove_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
        }
        if (dfc_increased) {
            s.deliberately_empty_count--;
            s.delib_empty_in_row--;
        }
        return;
    }

    coords next_immediate = s.next_position(position);
    if (((next_immediate.x != next_position.x) || (next_position.y != next_immediate.y)) ||
        (next_position.x != position.x)) {
        s.delib_empty_in_row = 0;
    }

    if (!s.could_be_better_than_best(next_position)) {
        if (tile_placed) {
            s.remove_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
        }
        if (dfc_increased) {
            s.deliberately_empty_count--;
            s.delib_empty_in_row --;
        }
        return;
    }

    find_cover(s, next_position, type1_len, HORIZONTAL, 1);
    find_cover(s, next_position, type2_len, HORIZONTAL, 2);

    find_cover(s, next_position, type1_len, VERTICAL, 1);
    find_cover(s, next_position, type2_len, VERTICAL, 2);

    find_cover(s, next_position, 0, LEAVE_EMPTY, 0);

    if (tile_placed) {
        s.remove_tile(tile_length, tile_type, position, tile_orientation);
        s.recalculate_cost();
    }

    if (dfc_increased) {
        s.deliberately_empty_count --;
        s.delib_empty_in_row --;
    }
}


// Only one worker executes this code sequentially
void solver::initiate_search(solution &s, coords initial_position) {
    s.recalculate_cost();
    s.compare_best();

    //cout << "Phase 1/5" << endl;
    find_cover(s, initial_position, type1_len, HORIZONTAL, 1);
    //cout << "Phase 2/5" << endl;
    find_cover(s, initial_position, type2_len, HORIZONTAL, 2);
    //cout << "Phase 3/5" << endl;
    find_cover(s, initial_position, type1_len, VERTICAL, 1);
    //cout << "Phase 4/5" << endl;
    find_cover(s, initial_position, type2_len, VERTICAL, 2);
    //cout << "Phase 5/5" << endl;
    find_cover(s, initial_position, 0, LEAVE_EMPTY, 0);
}
