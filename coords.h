//
// Created by vitek on 4.3.19.
//

#ifndef MI_PDP_COORDS_H
#define MI_PDP_COORDS_H

#include <iostream>

using namespace std;

class coords {

public:
    int x;
    int y;

    coords() {
        x=0;
        y=0;
    }

    coords(int x1, int y1) {
        x = x1;
        y = y1;
    }

    friend std::ostream& operator<< (std::ostream& stream, const coords& c){
        stream << "X: " << c.x << " Y: " << c.y << endl;
    }
};


#endif //MI_PDP_COORDS_H
