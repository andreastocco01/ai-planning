#include "../include/planning_task.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>

void PlanningTask::assert_version(std::ifstream &file) {
    std::string line;

    // translator version must be 3
    getline(file, line);
    assert(line == "begin_version");
    getline(file, line);
    assert(line == "3");
    getline(file, line);
    assert(line == "end_version");
}

void PlanningTask::get_metric(std::ifstream &file) {
    std::string line;

    // the metric must be 0 or 1
    getline(file, line);
    assert(line == "begin_metric");
    getline(file, line);
    int metric = std::stoi(line);
    assert(metric == 0 || metric == 1);
    this->metric = metric;
    getline(file, line);
    assert(line == "end_metric");
}

int PlanningTask::parse_from_file(std::string filename) {
    std::ifstream file (filename);

    if (!file.is_open()) {
        return 1;
    }

    assert_version(file);
    get_metric(file);

    file.close();
    return 0;
}
