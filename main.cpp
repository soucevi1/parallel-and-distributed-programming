#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>

#include "pole.h"
#include "solution.h"
#include "constants.h"
#include "solver.h"

using namespace std;

struct dimensions {
    int x;
    int y;
};

struct tile_info{
    int i1;
    int i2;
    int c1;
    int c2;
    int cn;
};

tile_info get_tile_info(ifstream ifstream);

string get_filename(int argc, char **argv) {
    int opt;
    string filename;

    if (argc != 3) {
        cout << "Wrong number of args (" << argc << ")" << endl;
        return "";
    }

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;

            case ':':
            case '?':
                filename = "";
                cout << "Wrong argument." << endl;
                break;

            default:
                filename = "";
                cout << "Unknown argument " << opt << endl;
        }
    }

    return filename;
}

dimensions get_dimensions(ifstream &f) {
    string line;
    dimensions d = {0, 0};

    getline(f, line);

    stringstream stream(line);
    stream >> d.x;
    stream >> d.y;
    return d;
}


tile_info get_tileinfo(ifstream &f) {

    string line;
    tile_info t = {0,0,0,0,0};

    getline(f, line);

    stringstream ss(line);
    ss >> t.i1;
    ss >> t.i2;
    ss >> t.c1;
    ss >> t.c2;
    ss >> t.cn;

    return t;
}


vector<coords> get_forbidden_fields(ifstream &f){
    string line;
    vector<coords> v;

    getline(f, line);
    stringstream ss(line);
    int f_count;

    ss >> f_count;

    for(int i=0; i<f_count; i++){

        getline(f, line);
        stringstream str(line);

        coords c(0,0);
        str >> c.y;
        str >> c.x;

        v.push_back(c);
    }

    return v;
}

int main(int argc, char **argv) {

    string filename = get_filename(argc, argv);
    if (filename.empty()) {
        return 1;
    }

    //cout << "Filename is: " << filename << endl;

    ifstream f;
    f.open(filename, ios_base::in);

    dimensions dims = {0,0};
    tile_info t_info = {0,0,0,0,0};

    unsigned long forbidden_count = 0;
    vector<coords> forbidden;

    if (f.is_open()) {
        dims = get_dimensions(f);
        t_info = get_tileinfo(f);
        forbidden = get_forbidden_fields(f);
        forbidden_count = forbidden.size();

    } else {
        cout << "Cannot open file" << endl;
        return -1;
    }

    f.close();

    pole p(dims.x, dims.y, forbidden_count, forbidden);

    solver s(p, t_info.i1, t_info.i2, t_info.c1, t_info.c2, t_info.cn);

    // MAIN FUNCTION CALL, measuring the time
    clock_t begin = clock();

    s.solve();

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    cout << "Finished in " << elapsed_secs << " seconds" << endl;

    return 0;
}
