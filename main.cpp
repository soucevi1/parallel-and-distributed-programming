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

struct tile_info {
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
    tile_info t = {0, 0, 0, 0, 0};

    getline(f, line);

    stringstream ss(line);
    ss >> t.i1;
    ss >> t.i2;
    ss >> t.c1;
    ss >> t.c2;
    ss >> t.cn;

    return t;
}


vector<coords> get_forbidden_fields(ifstream &f) {
    string line;
    vector<coords> v;

    getline(f, line);
    stringstream ss(line);
    int f_count;

    ss >> f_count;

    for (int i = 0; i < f_count; i++) {

        getline(f, line);
        stringstream str(line);

        coords c(0, 0);
        str >> c.y;
        str >> c.x;

        v.push_back(c);
    }

    return v;
}

vector<int> precalculate_problem_counts(int problems, int proc_count) {
    vector<int> prob_counts(proc_count);

    prob_counts[0] = 0;

    int assigned = 0;
    for (int i = 1; i < proc_count - 1; i++) {
        prob_counts[i] = (int) (problems / (proc_count - 1));
        assigned += prob_counts[i];
    }

    prob_counts[proc_count - 1] = problems - assigned;

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
    if (my_rank == 0) {

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

        vector<int> count_problems_to_send = precalculate_problem_counts(q_size, proc_count);

        cout << proc_count << " processes" << endl;


        for (int i = 1; i < proc_count; i++) {
            MPI_Send(&count_problems_to_send[i], 1, MPI_INT, i, COUNT_TAG, MPI_COMM_WORLD);
        }


        for (int i = 0; i < proc_count; i++) {
            for (int j = 0; j < count_problems_to_send[i]; j++) {

                solution sol = s.initial_solutions.front().starting_solution;
                coords pos = s.initial_solutions.front().position;
                s.initial_solutions.pop_front();

                comm_info c = comm_info(sol, pos, s.best_solution);

                string cs = c.serialize();

                MPI_Send(cs.c_str(), cs.length(), MPI_CHAR, i, WORK_TAG, MPI_COMM_WORLD);
            }
        }

        int working_slaves = proc_count - 1;
        vector<solution> slave_solutions;

        while (working_slaves > 0) {

            MPI_Status stat;
            MPI_Probe(MPI_ANY_SOURCE, FINAL_TAG, MPI_COMM_WORLD, &stat);

            int recv_len;
            MPI_Get_count(&stat, MPI_CHAR, &recv_len);
            char *m = new char[recv_len];

            MPI_Recv(m, recv_len, MPI_CHAR, MPI_ANY_SOURCE, FINAL_TAG, MPI_COMM_WORLD, &stat);

            solution recvd_sol = solution(m);

            slave_solutions.push_back(recvd_sol);
            working_slaves--;
        }

        /*
         *  Dokud jsi nedostal odpoved od vsech, prijimej zpravy
         *
         *  Kdyz prijmes reseni, odpovez STOP_TAGem
         *
         *  Az prijmes reseni od vsech, vyber z nich nejlepsi
         */

        solution the_best = slave_solutions[0];
        for (int i = 1; i < slave_solutions.size(); i++) {
            if (slave_solutions[i].cost > the_best.cost) {
                the_best = slave_solutions[i];
            }
        }
        cout << "The BEST solution is:" << endl;
        the_best.print_solution();


        // ------- SLAVE -----------------------------------------------
    } else {

        /*
         *  Nekonecny cyklus
         *
         *  Koukni, jestli je k dispozici zprava
         *  pokud ano:
         *      pokud je to STOP_TAG:
         *          breakni
         *      pokud COUNT_TAG:
         *          nacti si pocet
         *      jinak (WORK_TAG):
         *          prijmi podproblem
         *          pokud uz mas vsechny, breakni
         *
         */

        deque<comm_info> received_work;
        int solution_count = 0;
        int solutions_received = 0;

        while (true) {
            MPI_Status stat;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

            bool finish = false;
            switch (stat.MPI_TAG) {

                case STOP_TAG:
                    finish = true;
                    cout << my_rank << ": STOP" << endl;
                    break;

                case NEW_BEST_TAG:
                    /* Assign new best */
                    break;

                case COUNT_TAG:
                    MPI_Recv(&solution_count, 1, MPI_INT, MPI_ANY_SOURCE, COUNT_TAG, MPI_COMM_WORLD, &stat);
                    cout << my_rank << ": problems: " << solution_count << endl;
                    break;

                case WORK_TAG:
                    char *m;
                    int recvd;
                    MPI_Get_count(&stat, MPI_CHAR, &recvd);
                    m = new char[recvd];
                    MPI_Recv(m, recvd, MPI_CHAR, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
                    comm_info recvd_c = comm_info(string(m));
                    received_work.push_back(recvd_c);
                    solutions_received++;
                    if (solutions_received == solution_count) {
                        finish = true;
                    }
                    break;
            }

            if (finish) {
                break;
            }
        }

        solver s = solver(received_work);
        s.solve();
        string best_serial = s.best_solution.serialize();
        MPI_Send(best_serial.c_str(), best_serial.length(), MPI_CHAR, 0, FINAL_TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
