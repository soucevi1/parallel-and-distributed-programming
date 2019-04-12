//
// Created by vitek on 4.3.19.
//

#include <iostream>
#include <omp.h>
#include <mpi.h>
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
                                                                 free_cost(cn) {
    omp_init_lock(&best_lock);
}


solver::solver(deque<comm_info> received, int pc, int rank) : total_processes(pc), my_rank(rank) {

    best_solution = received[0].best;

    for (int i = 0; i < received.size(); i++) {
        initial_solution is;
        is.starting_solution = received[i].sol;
        is.position = received[i].position;
        initial_solutions.push_back(is);

        if (best_solution.cost < received[i].best.cost) {
            best_solution = received[i].best;
        }
    }

    type1_len = received[0].sol.type1_length;
    type2_len = received[0].sol.type2_length;

    type1_cost = received[0].sol.type1_cost;
    type2_cost = received[0].sol.type2_cost;
    free_cost = received[0].sol.empty_cost;

    best_updated = false;

    omp_init_lock(&best_lock);
}

void solver::generate_initial_solutions(int required_levels) {
    deque<initial_solution> q;

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

    q.push_back(is);

    int place_count = is.starting_solution.current_state.x_dim * is.starting_solution.current_state.y_dim;
    cout << "Places: " << place_count << endl;

    for (int i = 1; i < required_levels; i++) {

        // For each tree level,
        // generate subproblems from the whole queue

        deque<initial_solution> q_2;

        int queue_size = (int) q.size();

        if (queue_size > place_count * 10) {
            break;
        }

        for (int j = 0; j < queue_size; j++) {
            initial_solution tmp = q.front();
            q.pop_front();

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
                compare_with_best(tmp1.starting_solution, false);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push_back(tmp1);
            }

            // TYPE 2 horizontal
            if (tmp.starting_solution.check_if_tile_fits(type2_len, current_position, HORIZONTAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type2_len, 2, current_position, HORIZONTAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution, false);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push_back(tmp1);
            }

            // TYPE 1 vertical
            if (tmp.starting_solution.check_if_tile_fits(type1_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type1_len, 1, current_position, VERTICAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution, false);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push_back(tmp1);
            }

            // TYPE 2 vertical
            if (tmp.starting_solution.check_if_tile_fits(type2_len, current_position, VERTICAL)) {
                initial_solution tmp1 = tmp;
                tmp1.starting_solution.add_tile(type2_len, 2, current_position, VERTICAL);
                tmp1.starting_solution.recalculate_cost();
                compare_with_best(tmp1.starting_solution, false);
                next_position = tmp1.starting_solution.next_free_position(current_position);
                tmp1.position = next_position;
                tmp1.starting_solution.delib_empty_in_row = 0;
                q_2.push_back(tmp1);
            }

            // LEAVE EMPTY
            next_position = tmp.starting_solution.next_free_position(current_position);
            initial_solution tmp1 = tmp;
            tmp1.position = next_position;
            tmp1.starting_solution.delib_empty_in_row++;
            tmp1.starting_solution.deliberately_empty_count++;
            tmp1.starting_solution.recalculate_cost();
            compare_with_best(tmp1.starting_solution, false);
            if (!tmp1.starting_solution.can_fit_tile_behind(current_position) &&
                !tmp1.starting_solution.can_fit_tile_above(current_position)) {
                q_2.push_back(tmp1);
            }
        }

        q = q_2;
    }

    initial_solutions = q;
}

void solver::solve() {

    // Every worker gets one subproblem to solve
    int q_size = (int) initial_solutions.size();

#pragma omp parallel for default(shared) firstprivate(my_rank) firstprivate(total_processes) schedule(guided)
    for (int i = 0; i < q_size; i++) {
        initiate_search(initial_solutions[i].starting_solution, initial_solutions[i].position);
    }
}

void solver::find_cover(solution &s, coords &position, int tile_length, int tile_orientation, int tile_type) {

    bool tile_placed = false;
    bool dfc_increased = false;

#pragma omp master
    {
        check_best_message();

        bool bu;
#pragma omp atomic read
        bu = best_updated;

        if (bu) {
#pragma omp atomic write
            best_updated = false;

            send_best_message();
        }
    };

    // If supposed to place a tile
    if (tile_orientation != LEAVE_EMPTY) {

        if (s.check_if_tile_fits(tile_length, position, tile_orientation)) {
            // If tile fits, add it and recalculate cost
            s.add_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
            compare_with_best(s, true);

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
        s.deliberately_empty_count--;
        s.delib_empty_in_row--;
    }
}


// Only one worker executes this code sequentially
void solver::initiate_search(solution &s, coords initial_position) {
    s.recalculate_cost();
    compare_with_best(s, true);

    if (initial_position.x == -1) {
        return;
    }

    find_cover(s, initial_position, type1_len, HORIZONTAL, 1);
    find_cover(s, initial_position, type2_len, HORIZONTAL, 2);
    find_cover(s, initial_position, type1_len, VERTICAL, 1);
    find_cover(s, initial_position, type2_len, VERTICAL, 2);
    find_cover(s, initial_position, 0, LEAVE_EMPTY, 0);
}

void solver::compare_with_best(solution &sol, bool should_tell_others) {
    if (sol.cost > best_solution.cost) {
        omp_set_lock(&best_lock);
        if (sol.cost > best_solution.cost) {
            best_solution.cost = sol.cost;
            best_solution.current_state = sol.current_state;
            best_solution.type1_count = sol.type1_count;
            best_solution.type2_count = sol.type2_count;

            if (should_tell_others) {
#pragma omp atomic write
                best_updated = true;
            }
        }
        omp_unset_lock(&best_lock);
    }
}

void solver::check_best_message() {
    int recvd;
    int flag;

    MPI_Status stat;
    MPI_Iprobe(MPI_ANY_SOURCE, NEW_BEST_TAG, MPI_COMM_WORLD, &flag, &stat);

    if (!flag) {
        return;
    }

    MPI_Get_count(&stat, MPI_CHAR, &recvd);
    vector<char> m(recvd);
    MPI_Recv(&m[0], recvd, MPI_CHAR, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);

    string bs = string(m.begin(), m.end());

    solution best_candidate = solution(bs);

    compare_with_best(best_candidate, false);
}

void solver::send_best_message() {
    solution bsol;

    omp_set_lock(&best_lock);
    bsol = best_solution;
    omp_unset_lock(&best_lock);

    string bs = bsol.serialize();

    for (int i = 1; i < total_processes; i++) {

        if (i == my_rank) {
            continue;
        }
        MPI_Send(bs.c_str(), bs.length(), MPI_CHAR, i, NEW_BEST_TAG, MPI_COMM_WORLD);
    }
}

solver::solver() {}


