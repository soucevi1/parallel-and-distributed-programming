//
// Created by vitek on 4.3.19.
//

#include "pole.h"
#include <vector>
#include <iomanip>
#include "constants.h"
#include "coords.h"

using namespace std;

pole::pole(int x, int y, unsigned long forbidden_cnt, vector<coords> forb){
    x_dim = x;
    y_dim = y;
    forbidden_count = forbidden_cnt;
    forbidden = forb;

    map = new int* [x];

    for(int i=0; i<x; i++){
        map[i] = new int[y];
    }

    for(int i=0; i<x; i++){
        for(int j = 0; j<y; j++){
            map[i][j] = FREE_POS;
        }
    }

    for(int i =0; i<forbidden_count; i++){
        map[forbidden[i].x][forbidden[i].y] = FORB_POS;
    }
}

pole::~pole() {
    if(!map){
        return;
    }

    for(int i=0; i<x_dim; i++){
        delete [] map[i];
    }

    delete [] map;
}

bool pole::is_free(int x, int y) {
    return map[x][y] == FREE_POS;
}

void pole::print() {
    for(int i=0; i<3*x_dim+6; i++){
        cout << "-";
    }
    cout << endl;
    for(int i=0; i<x_dim; i++){
        for(int j=0; j<y_dim; j++){
            cout << setw(3);
            if(map[i][j] == FREE_POS) {
                cout << "  ." << " ";
            } else if(map[i][j] == FORB_POS){
                cout << "  X" << " ";
            } else {
                cout << map[i][j] << " ";
            }
        }
        cout << setw(1) << endl;
    }
    for(int i=0; i<3*x_dim+6; i++){
        cout << "-";
    }
    cout << endl;
}

pole::pole(const pole &p) {
    x_dim = p.x_dim;
    y_dim = p.y_dim;
    forbidden_count = p.forbidden_count;
    forbidden = p.forbidden;

    map = new int*[x_dim];

    for(int i=0; i<x_dim; i++){
        map[i] = new int[y_dim];
    }

    for(int i=0; i<x_dim; i++){
        for(int j = 0; j<y_dim; j++){
            map[i][j] = p.map[i][j];
        }
    }
}

pole & pole::operator=(const pole &p) {
    x_dim = p.x_dim;
    y_dim = p.y_dim;
    forbidden_count = p.forbidden_count;
    forbidden = p.forbidden;

    map = new int*[x_dim];

    for(int i=0; i<x_dim; i++){
        map[i] = new int[y_dim];
    }

    for(int i=0; i<x_dim; i++){
        for(int j = 0; j<y_dim; j++){
            map[i][j] = p.map[i][j];
        }
    }
}

pole::pole() {
    map = nullptr;
    x_dim = 0;
    y_dim = 0;
    forbidden_count = 0;
    forbidden = vector<coords> {};
}
