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
                                                                 free_cost(cn) {
    current_best = solution(p, 0,0, p.x_dim*p.y_dim, type1_cost, type2_cost, free_cost, 0);
    current_best.recalculate_cost();
}


queue<solution> solver::generate_initial_solutions() {
    queue<solution> q;

    int free_count = map.x_dim * map.y_dim - map.forbidden_count;

    solution s(map, 0, 0, free_count, type1_cost, type2_cost, free_cost, 0);
    s.recalculate_cost();
    q.push(s);

    return q;
}

void solver::solve() {
    queue<solution> q = generate_initial_solutions();

    for(int i=0; i<q.size(); i++){
        solution s = q.back();
        q.pop();

        coords pos = s.next_free_position(coords(0,0));
        s.print_map();


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


        find_cover(s, pos);

        cout << "The BEST solution is:" << endl;
        s.print_solution();

    }
}

void solver::find_cover(solution & s, coords position) {

    //cout << "pos: " << position.x << " " << position.y << endl;
    // TODO nefunguje
    // spatne urceni nejlepsiho reseni
    // hodne nizka cena
    // mozna neprojde vse, zkontrolovat, jake a kolik pozic projde

    s.recalculate_cost();
    if(s.cost > current_best.cost){
        current_best = s;
        cout << "Found better solution:" << endl;
        s.print_solution();
    }

    coords next_position = s.next_free_position(position);
    if(next_position.x == -1){
        return;
    }

    // Horizontal type 1
    if(s.check_if_tile_fits(type1_len, position, HORIZONTAL)) {
        s.add_tile(type1_len, 1, position, HORIZONTAL);
        next_position = s.next_free_position(position);
        if(next_position.x == -1){
            return;
        }
        find_cover(s, next_position);
        s.remove_tile(type1_len, 1, position, HORIZONTAL);
    }

    // Horizontal type 2
    if(s.check_if_tile_fits(type2_len, position, HORIZONTAL)) {
        s.add_tile(type2_len, 2, position, HORIZONTAL);
        next_position = s.next_free_position(position);
        if(next_position.x == -1){
            return;
        }
        find_cover(s, next_position);
        s.remove_tile(type2_len, 2, position, HORIZONTAL);
    }

    // Vertical type 1
    if(s.check_if_tile_fits(type1_len, position, VERTICAL)) {
        s.add_tile(type1_len, 1, position, VERTICAL);
        next_position = s.next_free_position(position);
        if(next_position.x == -1){
            return;
        }
        find_cover(s, next_position);
        s.remove_tile(type1_len, 1, position, VERTICAL);
    }

    // Vertical type 2
    if(s.check_if_tile_fits(type2_len, position, VERTICAL)) {
        s.add_tile(type2_len, 2, position, VERTICAL);
        next_position = s.next_free_position(position);
        if(next_position.x == -1){
            return;
        }
        find_cover(s, next_position);
        s.remove_tile(type2_len, 2, position, VERTICAL);
    }

    // Leave empty
    find_cover(s, next_position);


}
