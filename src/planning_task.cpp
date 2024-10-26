#include "../include/planning_task.h"
#include <fstream>
#include <iostream>
#include <cassert>

int PlanningTask::parse_from_file(std::string filename) {
    std::ifstream file (filename);

    if (!file.is_open()) {
        std::cout << "Error while opening the file" << std::endl;
    }

    std::string line;

    // translator version must be 3
    getline(file, line);
    assert(line == "begin_version");
    getline(file, line);
    assert(line == "3");
    getline(file, line);
    assert(line == "end_version");

    file.close();
    std::cout << "File parsed!" << std::endl;
    return 0;
}
