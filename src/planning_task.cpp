#include "../include/planning_task.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>

void PlanningTask::print_vars() {
    for (int i = 0; i < this->vars.size(); i++) {
        for (int j = 0; j < this->vars[i].sym_names.size(); j ++) {
            std::cout << this->vars[i].sym_names[j] << std::endl;
        }
        std::cout << std::endl;
    }
}

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

void PlanningTask::get_variables(std::ifstream &file) {
    std::string line;

    // number of variables
    getline(file, line);
    int n_vars = std::stoi(line);

    for (int i = 0; i < n_vars; i++) {
        getline(file, line);
        assert(line == "begin_variable");

        Variable var;
        getline(file, var.name);
        getline(file, line);
        var.axiom_layer = std::stoi(line);
        getline(file, line);
        var.range = stoi(line);

        for (int j = 0; j < var.range; j++) {
            getline(file, line);
            var.sym_names.push_back(line);
        }

        getline(file, line);
        assert(line == "end_variable");
        this->vars.push_back(var);
    }
}

int PlanningTask::parse_from_file(std::string filename) {
    std::ifstream file (filename);

    if (!file.is_open()) {
        return 1;
    }

    assert_version(file);
    get_metric(file);
    get_variables(file);

    file.close();
    return 0;
}
