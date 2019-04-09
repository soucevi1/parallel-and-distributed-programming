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

    unsigned long forbidden_count;
    vector<coords> forbidden;

    int **map;

    pole(int x, int y, unsigned long forb, vector<coords> f);
    pole(int x, int y, unsigned long forb);
    pole(const pole & p);
    pole();
    ~pole();

    pole & operator=(const pole & p);

    bool is_free(int x, int y);

    void print();

};


#endif //MI_PDP_POLE_H
