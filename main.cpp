#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <omp.h>
#include <mpi.h>

#include "pole.h"
#include "solution.h"
#include "constants.h"
#include "solver.h"
#include "comm_info.h"

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

vector<int> precalculate_problem_counts(int problems, int proc_count){
    vector<int> prob_counts(proc_count);

    prob_counts[0] = 0;

    int assigned = 0;
    for(int i=1; i<proc_count-1; i++){
        prob_counts[i] = (int)(problems/(proc_count-1));
        assigned += prob_counts[i];
    }

    prob_counts[proc_count-1] = problems - assigned;

    return prob_counts;
}

int main(int argc, char **argv) {

    int my_rank;
    dimensions dims = dimensions();
    tile_info t_info = tile_info();
    unsigned long forbidden_count = 0;
    vector<coords> forbidden;
    ifstream f;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // ------- MASTER -----------------------------------------------
    if(my_rank == 0){

        // Read input file
        string filename = get_filename(argc, argv);
        if (filename.empty()) {
            return 1;
        }
        f.open(filename, ios_base::in);
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

        // Generate initial subproblems
        pole p(dims.x, dims.y, forbidden_count, forbidden);
        solver s(p, t_info.i1, t_info.i2, t_info.c1, t_info.c2, t_info.cn);
        int required_levels = 4;
        s.generate_initial_solutions(required_levels);
        int q_size = (int) s.initial_solutions.size();
        cout << "Subproblems generated: " << q_size << endl;

        int proc_count;
        MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

        /*
         * Rozpocitej nagenerovana reseni,
         *
         * kazdemu procesu posli cislo, kolik reseni dostane a pak mu jich tolik posli
         */

        vector<int> count_problems_to_send = precalculate_problem_counts(q_size,proc_count);

        cout << proc_count << " processes" << endl;


        for(int i=1; i<proc_count; i++){
            MPI_Send(&count_problems_to_send[i], 1, MPI_INT, i, COUNT_TAG, MPI_COMM_WORLD);
        }

        for(int i=1; i<proc_count; i++){
            MPI_Send(&count_problems_to_send[i], 1, MPI_INT, i, STOP_TAG, MPI_COMM_WORLD);
        }



        MPI_Finalize();
        return 0;


        int proc_num = 1;
        for (int i = 0; i < q_size; i++) {
            solution sol = s.initial_solutions[i].starting_solution;
            coords position = s.initial_solutions[i].position;
            solution best = s.best_solution;

            comm_info c = comm_info(sol, position, best);
            string serialized = c.serialize();

            MPI_Send(serialized.c_str(), serialized.length(), MPI_CHAR, proc_num, WORK_TAG, MPI_COMM_WORLD);

            proc_num = ((proc_num + 1) % proc_count) + 1;
        }


        /*
         *  Dokud jsi nedostal odpoved od vsech, prijimej zpravy
         *
         *  Kdyz prijmes reseni, odpovez STOP_TAGem
         *
         *  Az prijmes reseni od vsech, vyber z nich nejlepsi
         */

        cout << "The BEST starting_solution is:" << endl;


    // ------- SLAVE -----------------------------------------------
    } else {

        /*
         *  Nekonecny cyklus
         *
         *  Koukni, jestli je k dispozici zprava
         *  pokud ano:
         *      pokud je to NEW_BEST_TAG:
         *          porovnej se svym nejlepsim, pripadne nahrad
         *      pokud je to STOP_TAG:
         *          breakni
         *      jinak (WORK_TAG):
         *          vyrob si solver
         *          nageneruj reseni pro paralelismus
         *          vyres
         *          odesli nejlepsi reseni masterovi
         *
         */

        while(true){
            int flag = 0;
            MPI_Status stat;
            //MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

            bool finish = false;
            switch(stat.MPI_TAG){
                case STOP_TAG:
                    finish = true;
                    cout << my_rank << ": STOP" << endl;
                    break;
                case NEW_BEST_TAG:
                    /* Assign new best */
                    break;
                case COUNT_TAG:
                    int m;
                    MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, COUNT_TAG, MPI_COMM_WORLD, &stat);
                    cout << my_rank << ": problems: " << m << endl;
                    break;
                case WORK_TAG:
                    break;
            }

            if(finish){
                break;
            }
        }

    }

    MPI_Finalize();

    return 0;
}
