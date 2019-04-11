//
// Created by vitek on 11.4.19.
//

#include <sstream>
#include "comm_info.h"

comm_info::comm_info(solution s, coords p, solution b) {
    sol = s;
    position = p;
    best = b;
}

string comm_info::serialize() {
    ostringstream os;

    os << sol.serialize();
    os << delim;
    os << position.x << " ";
    os << position.y;
    os << delim;
    os << best.serialize();

    return os.str();
}

comm_info::comm_info(string s) {

    size_t pos = s.find(delim);
    string sol_s = s.substr(0, pos);
    s.erase(0, pos + delim.length());

    pos = s.find(delim);
    string pos_s = s.substr(0, pos);
    s.erase(0, pos + delim.length());

    sol = solution(sol_s);

    best = solution(s);

    istringstream is(pos_s);
    is >> position.x;
    is >> position.y;
}
