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


queue<solution> solver::generate_initial_solutions() {
    queue<solution> q;

    int free_count = map.x_dim * map.y_dim - map.forbidden_count;

    solution s(map, 0, 0, free_count, type1_cost, type2_cost, free_cost, 0, type1_len, type2_len);
    s.recalculate_cost();
    q.push(s);

    return q;
}

void solver::solve() {
    queue<solution> q = generate_initial_solutions();

    for (int i = 0; i < q.size(); i++) {
        solution s = q.back();
        q.pop();

        coords pos = s.next_free_position(coords(0, 0));
        //s.print_map();


        /*
        for(int j=0; j<s.current_state.x_dim; j++){
            for(int k=0; k<s.current_state.y_dim; k++){
                s.current_state.map[j][k] = FORB_POS;
            }
        }
        s.print_map();

        coords c = s.next_free_position(coords(0,0));
        cout << c.x  << " " << c.y << endl;

        cout << s.current_state.is_free(0, 6) << endl;
        cout << s.current_state.is_free(8, 0) << endl;
        cout << s.current_state.is_free(8,6) << endl;
         */

        //s.print_solution();
        //int x = s.could_be_better_than_best(coords(8,1));


        initiate_search(s, pos);

        cout << "The BEST solution is:" << endl;
        s.best_solution.print_best();

    }
}

void solver::find_cover(solution &s, coords position, int tile_length, int tile_orientation, int tile_type) {

    bool tile_placed = false;

    // If supposed to place a tile
    if (tile_orientation != LEAVE_EMPTY) {

        if (s.check_if_tile_fits(tile_length, position, tile_orientation)) {
            // If tile fits, add it and recalculate cost
            s.add_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
            s.compare_best();
            tile_placed = true;

        } else {
            // Tile cannot be placed -- this branch ends
            return;
        }
    }

    coords next_position = s.next_free_position(position);

    // The search reached the end
    if(next_position.x == -1){
        // If tile was placed previously, remove it
        if(tile_placed) {
            s.remove_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
        }
        return;
    }

    if(! s.could_be_better_than_best(next_position)){
        if(tile_placed) {
            s.remove_tile(tile_length, tile_type, position, tile_orientation);
            s.recalculate_cost();
        }
        return;
    }

    find_cover(s, next_position, type1_len, HORIZONTAL, 1);
    find_cover(s, next_position, type2_len, HORIZONTAL, 2);

    find_cover(s, next_position, type1_len, VERTICAL, 1);
    find_cover(s, next_position, type2_len, VERTICAL, 2);

    find_cover(s, next_position, 0, LEAVE_EMPTY, 0);

    if(tile_placed){
        s.remove_tile(tile_length, tile_type, position, tile_orientation);
        s.recalculate_cost();
    }
}


// Only one worker executes this code sequentially
void solver::initiate_search(solution &s, coords initial_position) {
    s.recalculate_cost();
    s.compare_best();

    cout << "Phase 1/5" << endl;
    find_cover(s, initial_position, type1_len, HORIZONTAL, 1);
    cout << "Phase 2/5" << endl;
    find_cover(s, initial_position, type2_len, HORIZONTAL, 2);
    cout << "Phase 3/5" << endl;
    find_cover(s, initial_position, type1_len, VERTICAL, 1);
    cout << "Phase 4/5" << endl;
    find_cover(s, initial_position, type2_len, VERTICAL, 2);
    cout << "Phase 5/5" << endl;
    find_cover(s, initial_position, 0, LEAVE_EMPTY, 0);
}
