//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_SOLVER_H
#define MI_PDP_SOLVER_H

#include <vector>
#include "coords.h"

using namespace std;

class solver {
public:
    solver(int x, int y, unsigned long forb_cnt, vector<coords> forb, int i1, int i2, int c1, int c2, int cn);

    int dim_x;
    int dim_y;
    unsigned long forbidden_count;
    vector<coords> forbidden;

    int type1_len;
    int type2_len;

    int type1_cost;
    int type2_cost;
    int free_cost;

    void print();

};


#endif //MI_PDP_SOLVER_H