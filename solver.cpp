//
// Created by vitek on 4.3.19.
//

#include <iostream>
#include "solver.h"

using namespace std;

solver::solver(int x, int y, unsigned long forb_cnt, vector <coords> forb, int i1, int i2, int c1, int c2, int cn) : dim_x(x),
                                                                                                                     dim_y(y),
                                                                                                                     forbidden_count(forb_cnt),
                                                                                                                     forbidden(
                                                                                                                             forb),
                                                                                                                     type1_len(
                                                                                                                             i1),
                                                                                                                     type2_len(
                                                                                                                             i2),
                                                                                                                     type1_cost(
                                                                                                                             c1),
                                                                                                                     type2_cost(
                                                                                                                             c2),
                                                                                                                     free_cost(
                                                                                                                             cn) {}

void solver::print() {
    cout << "neco" << endl;
}