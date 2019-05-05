#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <omp.h>
#include <mpi.h>
#include <cstdlib>

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

string get_filename(int argc, char **argv) {
    int opt;
    string filename;

    if (argc != 5) {
        cout << "Wrong number of args (" << argc << ")" << endl;
        return "";
    }

    string threads;

    while ((opt = getopt(argc, argv, "f:t:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;

            case 't':
                threads = optarg;
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

    omp_set_num_threads(atoi(threads.c_str()));
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

solver generate_solver(string &filename) {

    dimensions dims = dimensions();
    tile_info t_info = tile_info();
    unsigned long forbidden_count = 0;
    vector<coords> forbidden;
    ifstream f;

    if (filename.empty()) {
        throw "Empty string was passed as filename";
    }
    f.open(filename, ios_base::in);
    if (f.is_open()) {
        dims = get_dimensions(f);
        t_info = get_tileinfo(f);
        forbidden = get_forbidden_fields(f);
        forbidden_count = forbidden.size();

    } else {
        throw "Cannot open file " + filename;
    }
    f.close();

    // Generate initial subproblems
    pole p(dims.x, dims.y, forbidden_count, forbidden);
    solver s(p, t_info.i1, t_info.i2, t_info.c1, t_info.c2, t_info.cn);
    return s;
}

int get_idle_slave(vector<bool> slave_work) {
    int worker = 0;
    for (int i = 1; i < slave_work.size(); i++) {
        if (slave_work[i] == false) {
            worker = i;
            break;
        }
    }
    return worker;
}




solution receive_solution(MPI_Status &stat){
    int recvd;
    MPI_Get_count(&stat, MPI_CHAR, &recvd);
    vector<char> m(recvd);
    MPI_Recv(&m[0], recvd, MPI_CHAR, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
    solution recvd_solution = solution(string(m.begin(), m.end()));
    return recvd_solution;
}

void send_work_to_slave(int idle_slave, solver & s, solution & master_best){
    solution sol = s.initial_solutions.front().starting_solution;
    coords pos = s.initial_solutions.front().position;
    s.initial_solutions.pop_front();

    comm_info c = comm_info(sol, pos, master_best);

    string cs = c.serialize();

    MPI_Send(cs.c_str(), cs.length(), MPI_CHAR, idle_slave, WORK_TAG, MPI_COMM_WORLD);
}

vector<bool> stop_idle_slaves(int proc_count, vector<bool> & slave_working){
    vector<bool>slaves_stopped(proc_count);
    for (int i = 1; i < slave_working.size(); i++) {
        if (!slave_working[i]) {
            int k=1;
            MPI_Send(&k, 1, MPI_INT, i, STOP_TAG, MPI_COMM_WORLD);
            slaves_stopped[i] = true;
        } else{
            slaves_stopped[i] = false;
        }
    }
    return slaves_stopped;
}

comm_info receive_comm_info(MPI_Status & stat){
    int recvd;
    MPI_Get_count(&stat, MPI_CHAR, &recvd);
    vector<char> m(recvd);
    MPI_Recv(&m[0], recvd, MPI_CHAR, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);

    comm_info recvd_c = comm_info(string(m.begin(), m.end()));

    return recvd_c;
}

int main(int argc, char **argv) {

    int my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int proc_count;
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

    string filename = get_filename(argc, argv);

    // ================================ MASTER ======================================
    if (my_rank == 0) {

        // TIME MEASURE BEGINS
        double begin_measure = MPI_Wtime();

        // Read input file

        solver s;
        try {
            s = generate_solver(filename);
        } catch (const char *e) {
            cout << e << endl;
            return 1;
        }

        int required_levels = 4;

        // Generate subsolutions for the slaves
        s.generate_solutions_for_slaves(required_levels);
        int q_size = (int) s.initial_solutions.size();
        cout << "Master: subproblems generated: " << q_size << endl;

        vector<bool> slave_working(proc_count, false);
        slave_working[0] = true;

        solution master_best = s.best_solution;

        int received_solutions = 0;
        int sent_work = 0;

        // Send all the subproblems to the slaves
        while (!s.initial_solutions.empty()) {

            MPI_Status stat;
            int flag;

            // Is message available?
            MPI_Iprobe(MPI_ANY_SOURCE, FINAL_TAG, MPI_COMM_WORLD, &flag, &stat);

            // Message available -- read the message
            if (flag) {
                solution recvd_solution = receive_solution(stat);

                if (recvd_solution.cost > master_best.cost) {
                    master_best = recvd_solution;
                }

                slave_working[stat.MPI_SOURCE] = false;
                received_solutions++;
                continue;
            }

            // Is there a worker that has nothing to do?
            int idle_slave = get_idle_slave(slave_working);
            if (idle_slave == 0) {
                continue;
            }

            // Send subproblem to an idle slave
            send_work_to_slave(idle_slave, s, master_best);
            slave_working[idle_slave] = true;
            sent_work++;
        }

        // All subproblems sent, tell idle slaves to exit
        vector<bool> slaves_stopped = stop_idle_slaves(proc_count, slave_working);

        // Wait for the last solutions
        while (received_solutions < sent_work) {
            MPI_Status stat;
            MPI_Probe(MPI_ANY_SOURCE, FINAL_TAG, MPI_COMM_WORLD, &stat);

            solution recvd_sol = receive_solution(stat);

            received_solutions++;
            slave_working[stat.MPI_SOURCE] = false;

            if (recvd_sol.cost > master_best.cost) {
                master_best = recvd_sol;
            }
        }

        // Stop all the remaining slaves now
        slaves_stopped = stop_idle_slaves(proc_count, slave_working);

        cout << "The BEST solution is:" << endl;
        master_best.print_solution();

        // TIME MEASURE ENDS
        double end_measure = MPI_Wtime();
        double elapsed_secs = end_measure - begin_measure;

        cout << "Finished in " << elapsed_secs << " seconds." << endl;


        // ================================ SLAVE ======================================
    } else {

        int work_id = 0;

        // Work until told otherwise
        while (true) {
            MPI_Status stat;

            // Wait for a message
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

            bool finish = false;
            int tag = stat.MPI_TAG;
            switch (tag) {

                case STOP_TAG: {
                    // Master wants the slave to stop
                    int k;
                    MPI_Recv(&k, 1, MPI_INT, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
                    finish = true;
                }
                    break;

                case WORK_TAG: {
                    // Master sends new work
                    comm_info recvd_c = receive_comm_info(stat);
                    solver s = solver(recvd_c, proc_count, my_rank);
                    s.solve();
                    string best_serial = s.best_solution.serialize();
                    MPI_Send(best_serial.c_str(), best_serial.length(), MPI_CHAR, 0, FINAL_TAG, MPI_COMM_WORLD);
                    work_id++;
                }
                    break;

                case NEW_BEST_TAG: {
                    // Idle slave received a message with someone else's best solution
                    // Receive the message and throw it away
                    solution sol = receive_solution(stat);
                }
                    break;

                default: {
                    // Slave received a message it does not understand
                    cout << my_rank << ": Got unexpected message with tag " << stat.MPI_TAG << " from "
                         << stat.MPI_SOURCE << " error: " << stat.MPI_ERROR << endl;
                }
            }

            if (finish) {
                break;
            }
        }
    }

    MPI_Finalize();

    return 0;
}
