#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "pole.cpp"
#include "solution.cpp"
#include "constants.h"

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
        str >> c.x;
        str >> c.y;

        v.push_back(c);
    }

    return v;
}

int main(int argc, char **argv) {

    string filename = get_filename(argc, argv);
    if (filename.empty()) {
        return 1;
    }

    cout << "Filename is: " << filename << endl;

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

    /*
    cout << "X: " << dims.x << ", Y: " << dims.y << endl;
    cout << "i1: " << t_info.i1 << ", i2: " << t_info.i2 << endl;
    cout << "c1: " << t_info.c1 << ", c2: " << t_info.c2 << ", cn: " << t_info.cn << endl;
    cout << "Forbidden: " << forbidden_count << endl;
    for(unsigned long i=0; i<forbidden_count; i++){
        cout << "[" << forbidden[i].x << ", " << forbidden[i].y << "]" << endl;
    }
     */

    f.close();


    // test pole
    vector<coords> forb = {coords(2,4), coords(3,3)};
    pole p(7, 9, 2, forb);
    //p.print();
    cout << "free " << p.is_free(1,1) << ", " << p.is_free(2,4) << endl;

    solution s(p, coords(1,1), 0, 0, 61, 15);
    s.print_map();
    cout << "fit " << s.check_if_tile_fits(3, coords(15, 18), HORIZONTAL) << endl;

    solution s2 = s;
    s2.add_tile(3, coords(2,0), VERTICAL);
    s2.print_map();

    solution s3 = s2;
    s3.add_tile(6, coords(0,0), HORIZONTAL);
    cout << endl;
    s3.print_map();




    /*
     * Vytvorit prazdnou mapu - nejlepsi dosazene reseni, pamatovat si jeji cenu
     *
     * Nagenerovat si podstromy
     *      - vytvorit 5 map (na prvni volne policko polozit 4 zpusoby a nepolozit)
     *      - spocitat ceny, porovnat s nejlepsim
     *      - ulozit do fronty
     *
     * Dokud fronta neprazdna: Vzit z fronty mapu
     *
     * Zacit na podstromu funkci DFS
     *      - z uzlu (stavu mapy) pro kazde nasledujici policko:
     *          - kdyz nemuzu pridat dlazdici, koncim
     *          - jinak volat funkci DFS s mapou, kam pridam jednu dlazdici na aktualni nasledujici policko
     *          - u kazdeho stavu pocitat cenu a porovnavat s nejlepsi
     *
     * Pamatovat si aktualni cislo policka, aby sel udelat cyklus testovani policek od aktualniho do konce
     */


    return 0;
}
