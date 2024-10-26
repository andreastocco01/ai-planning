#include <iostream>
#include "include/planning_task.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Missing file to parse" << std::endl;
        return 1;
    }

    PlanningTask pt;
    pt.parse_from_file(argv[1]);
    return 0;
}
