#include <iostream>
#include <string>

#include "include/planning_task.h"
#include "include/planning_task_parser.h"
#include "include/planning_task_utils.h"

void print_usage(std::string executable) {
    std::cerr << "Usage: " << executable
              << " --from-file <file_name> --alg <alg_code> --seed <int> "
                 "[--time-limit <int>] --debug <bool>"
              << std::endl;
    std::cerr << "Supported alg_code are:" << std::endl
              << "0: random" << std::endl
              << "1: greedy" << std::endl
              << "2: h_add" << std::endl
              << "3: h_max" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 9) {
        print_usage(argv[0]);
        return 1;
    }

    bool from_file_flag = false;
    bool alg_flag = false;
    bool seed_flag = false;
    bool time_limit_flag = false;
    bool debug_flag = false;
    std::string file_name;
    int alg;
    int seed;
    int time_limit;
    int debug;
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
        if (arg == "--time-limit") {
            time_limit_flag = true;
            time_limit = std::stoi(argv[++i]);
        }
        if (arg == "--debug") {
            debug_flag = true;
            debug = std::stoi(argv[++i]);
        }
    }

    if (!(from_file_flag && alg_flag && seed_flag && debug_flag) || alg < 0 ||
        alg > 6) {
        print_usage(argv[0]);
        return 1;
    }

    if (!time_limit_flag) time_limit = -1;

    PlanningTaskParser parser;
    PlanningTask pt = parser.parse_from_file(file_name);
    std::cout << "File " << file_name << " parsed!" << std::endl << std::endl;
    std::cout << "############ File structure #############" << std::endl;
    PlanningTaskUtils::print_structure(pt);

    std::cout << std::endl << "Running algorithm: ";

    switch (alg) {
        case 0:
            std::cout << "random" << std::endl;
            break;
        case 1:
            std::cout << "greedy" << std::endl;
            break;
        case 2:
            std::cout << "hmax_prof" << std::endl;
            break;
        case 4:
            std::cout << "hadd_rec" << std::endl;
            break;
        case 5:
            std::cout << "hmax_rec" << std::endl;
            break;
        case 6:
            std::cout << "hmax_it" << std::endl;
            break;
    }

    if (!pt.solve(seed, alg, debug, time_limit)) {
        std::cout << std::endl
                  << "############### Solution ###############" << std::endl;
        pt.print_solution();
    }

    return 0;
}
