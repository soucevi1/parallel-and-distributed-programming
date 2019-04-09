//
// Created by vitek on 4.3.19.
//

#include "solution.h"
#include "constants.h"
#include "coords.h"
#include <limits.h>
#include <sstream>

solution::solution(pole map, int t1_count, int t2_count, int free_count, int t1_cost, int t2_cost, int free_cost,
                   int cost, int t1_len, int t2_len) : current_state(map),
                                                       type1_count(t1_count),
                                                       type1_cost(t1_cost),
                                                       type2_count(t2_count),
                                                       type2_cost(t2_cost),
                                                       empty_count(free_count),
                                                       empty_cost(free_cost),
                                                       deliberately_empty_count(0),
                                                       cost(cost),
                                                       type1_length(t1_len),
                                                       type2_length(t2_len),
                                                       delib_empty_in_row(0) {
    shortest_tile_length = type1_length > type2_length ? type2_length : type1_length;
};

void solution::recalculate_cost() {
    cost = type1_cost * type1_count + type2_cost * type2_count + empty_count * empty_cost;
}

bool solution::check_if_tile_fits(int length, coords &pos, int direction) {

    if (direction == VERTICAL) {

        if (current_state.x_dim < pos.x + length) {
            return false;
        }

        for (int i = 0; i < length; i++) {
            if (current_state.map[pos.x + i][pos.y] != EMPTY_POS) {
                return false;
            }
        }

    } else if (direction == HORIZONTAL) {

        if (current_state.y_dim < pos.y + length) {
            return false;
        }

        for (int i = 0; i < length; i++) {
            if (current_state.map[pos.x][pos.y + i] != EMPTY_POS) {
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
    empty_count -= length;
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
            current_state.map[pos.x + i][pos.y] = EMPTY_POS;
        }

    } else if (direction == HORIZONTAL) {

        for (int i = 0; i < length; i++) {
            current_state.map[pos.x][pos.y + i] = EMPTY_POS;
        }
    }

    if (type == 1) {
        type1_count--;
    } else {
        type2_count--;
    }
    empty_count += length;
}

solution::solution() {
    current_state = pole();
    type1_count = 0;
    type1_cost = 0;
    type2_count = 0;
    type2_cost = 0;
    empty_cost = 0;
    empty_count = 0;
    delib_empty_in_row = 0;
    deliberately_empty_count = 0;
}

void solution::print_solution() {
    print_map();
    cout << "Cost: " << cost << endl;
    cout << "Length " << type1_length << ": " << type1_count << endl;
    cout << "Length " << type2_length << ": " << type2_count << endl;

    vector<coords> uncovered;
    int uncovered_count = 0;

    for(int i=0; i<current_state.x_dim; i++){
        for(int j=0; j<current_state.y_dim; j++){
            if(current_state.map[i][j] == EMPTY_POS){
                uncovered_count ++;
                uncovered.emplace_back(coords(i,j));
            }
        }
    }

    cout << "Uncovered: " << uncovered_count << endl;
    for (auto &i : uncovered) {
        cout << "    [" << i.x << ", " << i.y << "]" << endl;
    }
    cout << endl;
}

void solution::compare_best(solution &best_sol) {
    if (cost > best_sol.cost) {
        best_sol.cost = cost;
        best_sol.current_state = current_state;
        best_sol.type1_count = type1_count;
        best_sol.type2_count = type2_count;
    }
}

bool solution::could_be_better_than_best(coords &position, solution &best_sol) {

    // Hypothetical reachable cost
    int hypothetical_cost = cost + eval(empty_count);

    // Need to remove the empty penalization
    hypothetical_cost -= empty_cost * empty_count;

    // Add the empty penalization for fields
    // that were left empty on purpose
    hypothetical_cost += empty_cost * deliberately_empty_count;

    return hypothetical_cost > best_sol.cost;
}

int solution::eval(int number) {
    // vraci maximalni cenu pro "number" nevyresenych policek
    // returns the maximal price for the "number" of unsolved squares
    int i, x;
    int max = type2_cost * (number / type2_length);
    int zb = number % type2_length;
    max += type1_cost * (zb / type1_length);
    zb = zb % type1_length;
    max += zb * empty_cost;
    for (i = 0; i < (number / type2_length); i++) {
        x = type2_cost * i;
        zb = number - i * type2_length;
        x += type1_cost * (zb / type1_length);
        zb = zb % type1_length;
        x += zb * empty_cost;
        if (x > max) max = x;
    }
    return max;
}

bool solution::can_fit_tile_behind(coords &position) {
    return shortest_tile_length <= delib_empty_in_row;
}

bool solution::can_fit_tile_above(coords &position) {
    if ( (position.x + 1) - shortest_tile_length < 0) {
        return false;
    }

    for (int i = position.x; i > position.x - shortest_tile_length; i--) {
        if (current_state.map[i][position.y] != EMPTY_POS) {
            return false;
        }
    }
    return true;
}

string solution::serialize() {
    ostringstream os;

    os << cost << " ";
    os << type1_length << " " << type1_count << " " << type1_cost << " ";
    os << type2_length << " " << type2_count << " " << type2_cost << " ";
    os << empty_count << " " << empty_cost << " " << deliberately_empty_count << " " << delib_empty_in_row << " ";

    os << current_state.x_dim << " " << current_state.y_dim << " " << current_state.forbidden_count << " ";

    for(int i=0; i<current_state.x_dim; i++){
        for(int j=0; j<current_state.y_dim; j++){
            os << current_state.map[i][j] << " ";
        }
    }

    return os.str();
}

solution::solution(const string& serialized) {
    // Deserialize string to a new solution
    istringstream is(serialized);


    is >> cost;

    is >> type1_length;
    is >> type1_count;
    is >> type1_cost;
    is >> type2_length;
    is >> type2_count;
    is >> type2_cost;

    is >> empty_count;
    is >> empty_cost;
    is >> deliberately_empty_count;
    is >> delib_empty_in_row;

    int x, y, fc;
    is >> x;
    is >> y;
    is >> fc;

    current_state = pole(x, y, fc);

    for(int i=0; i<current_state.x_dim; i++){
        for(int j=0; j<current_state.y_dim; j++){
            is >> current_state.map[i][j];
            if(current_state.map[i][j] == FORB_POS){
                current_state.forbidden.emplace_back(i,j);
            }
        }
    }

}

