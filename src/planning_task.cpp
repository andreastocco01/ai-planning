#include "../include/planning_task.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <ostream>
#include <string>
#include <sstream>

void PlanningTask::print_vars() {
    for (int i = 0; i < this->vars.size(); i++) {
        for (int j = 0; j < this->vars[i].sym_names.size(); j ++) {
            std::cout << this->vars[i].sym_names[j] << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTask::print_facts() {
    for (int i = 0; i < this->mutexes.size(); i++) {
        for (int j = 0; j < this->mutexes[i].n_facts; j++) {
            std::cout << this->mutexes[i].facts[j].var_idx << " " << this->mutexes[i].facts[j].var_val << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTask::assert_version(std::ifstream &file) {
    std::string line;

    // the translator version must be 3
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

void PlanningTask::get_facts(std::ifstream &file) {
    std::string line;

    // number of mutex groups
    getline(file, line);
    int n_mutex = stoi(line);

    for (int i = 0; i < n_mutex; i++) {
        getline(file, line);
        assert(line == "begin_mutex_group");

        MutexGroup mutex;
        getline(file, line);
        mutex.n_facts = std::stoi(line);

        for (int i = 0; i < mutex.n_facts; i++) {
            Fact fact;
            getline(file, line);

            std::istringstream iss(line);
            std::string tok;

            int count = 0;
            while(iss >> tok) {
                if (count == 0)
                    fact.var_idx = std::stoi(tok);
                else
                    fact.var_val = std::stoi(tok);
                count++;
            }

            mutex.facts.push_back(fact);
        }

        getline(file, line);
        assert(line == "end_mutex_group");
        this->mutexes.push_back(mutex);
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
    get_facts(file);

    file.close();
    return 0;
}
