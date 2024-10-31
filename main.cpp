#include <iostream>
#include "include/planning_task.h"
#include "include/planning_task_parser.h"
#include "include/planning_task_utils.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Missing file to parse" << std::endl;
        return 1;
    }

    PlanningTaskParser parser;
    PlanningTask pt = parser.parse_from_file(argv[1]);
    std::cout << "File parsed!" << std::endl;

    return 0;
}
