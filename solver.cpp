//
// Created by vitek on 4.3.19.
//

#include <iostream>
#include <omp.h>
#include "solver.h"
#include "constants.h"
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

    int free_count = (int) (map.x_dim * map.y_dim - map.forbidden_count);

    solution sol(map, 0, 0, free_count, type1_cost, type2_cost, free_cost, 0, type1_len, type2_len);
    sol.recalculate_cost();

    coords current_position(0, 0);
    if (!sol.current_state.is_free(current_position.x, current_position.y)) {
        current_position = sol.next_free_position(current_position);
    }

    initial_solution is;
    is.starting_solution = sol;
    is.position = current_position;

    best_solution = sol;

    q.push(is);

    for (int i = 1; i < required_levels; i++) {

        // For each tree level,
        // generate subproblems from the whole queue

        queue<initial_solution> q_2;

        int queue_size = (int) q.size();

        for (int j = 0; j < queue_size; j++) {
            initial_solution tmp = q.front();
            q.pop();

            current_position = tmp.position;
            if (current_position.x == -1) {
                continue;
            }

            coords next_position;

            // TYPE 1 horizontal
            if (tmp.starting_solution.check_if_tile_fits(type1_len, current_position, HORIZONTAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type1_len, 1, current_position, HORIZONTAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push(tmp1);
            }

            // TYPE 2 horizontal
            if (tmp.starting_solution.check_if_tile_fits(type2_len, current_position, HORIZONTAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type2_len, 2, current_position, HORIZONTAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push(tmp1);
            }

            // TYPE 1 vertical
            if (tmp.starting_solution.check_if_tile_fits(type1_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type1_len, 1, current_position, VERTICAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push(tmp1);
            }

            // TYPE 2 vertical
            if (tmp.starting_solution.check_if_tile_fits(type2_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type2_len, 2, current_position, VERTICAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push(tmp1);
            }

            // LEAVE EMPTY
            next_position = tmp.starting_solution.next_free_position(current_position);
            initial_solution tmp1 = tmp;
            tmp1.position = next_position;
            tmp1.starting_solution.delib_empty_in_row++;
            tmp1.starting_solution.deliberately_empty_count++;
            tmp1.starting_solution.recalculate_cost();
            compare_with_best(tmp1.starting_solution);
            if (!tmp1.starting_solution.can_fit_tile_behind(current_position) &&
                !tmp1.starting_solution.can_fit_tile_above(current_position)) {
                q_2.push(tmp1);
            }
        }

        q = q_2;
    }

    initial_solutions = q;
}

void solver::solve() {

    // TODO - predelat Queue na vector -- nutno mit moznost iterovat

    int required_number_of_levels = 1;

    generate_initial_solutions(required_number_of_levels);

    // Every worker gets one subproblem to solve
    int q_size = (int) initial_solutions.size();

    for (int i = 0; i < q_size; i++) {
        initial_solution s = initial_solutions.front();
        initial_solutions.pop();

#pragma omp parallel default(shared)
        {
#pragma omp single
            //cout << "Threads: " <<omp_get_num_threads() << endl;
            initiate_search(s.starting_solution, s.position);
        };
    }

    cout << "The BEST solution is:" << endl;
    best_solution.print_solution();
}

void solver::find_cover(solution s, const coords &position, int tile_length, int tile_orientation, int tile_type) {

    bool tile_placed = false;
    bool dfc_increased = false;

    // If supposed to place a tile
    if (tile_orientation != LEAVE_EMPTY) {

        if (s.check_if_tile_fits(tile_length, position, tile_orientation)) {
            // If tile fits, add it and recalculate cost
            s.add_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
            compare_with_best(s);

            tile_placed = true;
            s.delib_empty_in_row = 0;

        } else {
            // Tile cannot be placed -- this branch ends
            return;
        }
    } else {
        s.deliberately_empty_count++;
        s.delib_empty_in_row++;
        dfc_increased = true;
        if (s.can_fit_tile_behind(position) || s.can_fit_tile_above(position)) {
            s.deliberately_empty_count--;
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

    if (!s.could_be_better_than_best(next_position, best_solution)) {
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

    int id =  s.current_state.y_dim * position.x + position.y;

    bool task_condition = id < TASK_LEVEL_THRESHOLD;

#pragma omp task if (task_condition)
        find_cover(s, next_position, type1_len, HORIZONTAL, 1);
#pragma omp task if (task_condition)
        find_cover(s, next_position, type2_len, HORIZONTAL, 2);
#pragma omp task if (task_condition)
        find_cover(s, next_position, type1_len, VERTICAL, 1);
#pragma omp task if (task_condition)
        find_cover(s, next_position, type2_len, VERTICAL, 2);
//#pragma omp task if (task_condition)
        find_cover(s, next_position, 0, LEAVE_EMPTY, 0);
//#pragma omp taskwait


    if (tile_placed) {
        s.remove_tile(tile_length, tile_type, position, tile_orientation);
        s.recalculate_cost();
    }

    if (dfc_increased) {
        s.deliberately_empty_count--;
        s.delib_empty_in_row--;
    }
}


// Only one worker executes this code sequentially
void solver::initiate_search(solution s, coords initial_position) {
    s.recalculate_cost();
    s.compare_best(best_solution);

    if (initial_position.x == -1) {
        return;
    }

#pragma omp task
        find_cover(s, initial_position, type1_len, HORIZONTAL, 1);
#pragma omp task
        find_cover(s, initial_position, type2_len, HORIZONTAL, 2);
#pragma omp task
        find_cover(s, initial_position, type1_len, VERTICAL, 1);
#pragma omp task
        find_cover(s, initial_position, type2_len, VERTICAL, 2);
#pragma omp task
        find_cover(s, initial_position, 0, LEAVE_EMPTY, 0);

//#pragma omp taskwait
}

void solver::compare_with_best(solution &sol) {
    if (sol.cost > best_solution.cost) {
#pragma omp critical
        {
            if (sol.cost > best_solution.cost) {
                best_solution.cost = sol.cost;
                best_solution.current_state = sol.current_state;
                best_solution.type1_count = sol.type1_count;
                best_solution.type2_count = sol.type2_count;
            }
        }
        // best_solution.print_solution();
        // cout << "=====================================" <<endl;
    }
}
