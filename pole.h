//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_POLE_H
#define MI_PDP_POLE_H


#include "coords.h"
#include <vector>

using namespace std;

class pole {

public:

    int x_dim;
    int y_dim;

    int forbidden_count;
    vector<coords> forbidden;

    int **map;

    pole(int x, int y, int forb, vector<coords> f);
    pole(const pole & p);
    ~pole();

    pole & operator=(const pole & p);

    bool is_free(int x, int y);

    void print();

};


#endif //MI_PDP_POLE_H
