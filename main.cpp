#include <iostream>
#include <string>
#include "include/planning_task.h"
#include "include/planning_task_parser.h"
#include "include/planning_task_utils.h"

void print_usage(std::string executable) {
    std::cerr << "Usage: " << executable << " --from-file <file_name> --alg <alg_code> [--seed <int>]" << std::endl;
    std::cerr << "Supported alg_code are:" << std::endl << "0: brute_force (seed required)" << "1: greedy (seed required)" << std::endl;
}

int main(int argc, char** argv) {

    if (argc < 5) {
        print_usage(argv[0]);
        return 1;
    }

    bool from_file_flag = false;
    bool alg_flag = false;
    bool seed_flag = false;
    std::string file_name;
    int alg;
    int seed;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--from-file") {
            from_file_flag = true;
            file_name = argv[++i];
        }
        if (arg == "--alg") {
            alg_flag = true;
            alg = std::stoi(argv[++i]);
        }
        if (arg == "--seed") {
            seed_flag = true;
            seed = std::stoi(argv[++i]);
        }
    }

    if (!(from_file_flag && alg_flag)) {
        print_usage(argv[0]);
        return 1;
    }

    PlanningTaskParser parser;
    PlanningTask pt = parser.parse_from_file(file_name);
    std::cout << "File parsed!" << std::endl;

    if (alg == 0 && seed_flag) {
        pt.brute_force(seed);
        pt.print_solution();
    } else if (alg == 1 && seed_flag) {
        pt.greedy(seed);
        pt.print_solution();
    } else if (alg == 2 && seed_flag) {
        pt.solve(seed);
        pt.print_solution();
    } else {
        print_usage(argv[0]);
        return 1;
    }
    PlanningTaskUtils::print_structure(pt);

    return 0;
}
