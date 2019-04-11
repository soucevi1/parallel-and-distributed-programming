//
// Created by vitek on 11.4.19.
//

#ifndef MI_PDP_COMM_INFO_H
#define MI_PDP_COMM_INFO_H


#include "solution.h"

class comm_info {
public:
    solution sol;
    coords position;
    solution best;

    string delim = "@@@";

    comm_info(solution s, coords p, solution b);
    comm_info(string s);

    string serialize();

};


#endif //MI_PDP_COMM_INFO_H
