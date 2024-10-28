#include "../include/planning_task.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <ostream>
#include <string>
#include <sstream>

void PlanningTask::print_vars() {
    for (int i = 0; i < this->n_vars; i++) {
        for (int j = 0; j < this->vars[i].range; j ++) {
            std::cout << this->vars[i].sym_names[j] << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTask::print_facts() {
    for (int i = 0; i < this->n_mutex; i++) {
        for (int j = 0; j < this->mutexes[i].n_facts; j++) {
            std::cout << this->mutexes[i].facts[j].var_idx << " " << this->mutexes[i].facts[j].var_val << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTask::print_initial_state() {
    for (int i = 0; i < this->n_vars; i++) {
        std::cout << this->initial_state[i] << std::endl;
    }
    std::cout << std::endl;
}

void PlanningTask::print_goal() {
    for (int i = 0; i < this->n_goals; i++) {
        std::cout << this->goal_state[i].var_idx << " " << this->goal_state[i].var_val << std::endl;
    }
    std::cout << std::endl;
}

void PlanningTask::print() {
    std::cout << "Metric: " << this->metric << std::endl;
    std::cout << std::endl;
    std::cout << "Variables:" << std::endl;
    print_vars();
    std::cout << "Facts:" << std::endl;
    print_facts();
    std::cout << "Initial state" << std::endl;
    print_initial_state();
    std::cout << "Goal state" << std::endl;
    print_goal();
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
    this->n_vars = std::stoi(line);

    for (int i = 0; i < this->n_vars; i++) {
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

Fact PlanningTask::parse_fact(std::string line){
    Fact fact;
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
    return fact;
}

void PlanningTask::get_facts(std::ifstream &file) {
    std::string line;

    // number of mutex groups
    getline(file, line);
    this->n_mutex = stoi(line);

    for (int i = 0; i < this->n_mutex; i++) {
        getline(file, line);
        assert(line == "begin_mutex_group");

        MutexGroup mutex;
        getline(file, line);
        mutex.n_facts = std::stoi(line);

        for (int i = 0; i < mutex.n_facts; i++) {
            getline(file, line);
            mutex.facts.push_back(parse_fact(line));
        }

        getline(file, line);
        assert(line == "end_mutex_group");
        this->mutexes.push_back(mutex);
    }
}

void PlanningTask::get_initial_state(std::ifstream &file) {
    std::string line;
    getline(file, line);

    assert(line == "begin_state");

    for (int i = 0; i < this->n_vars; i++) {
        getline(file, line);
        int var = std::stoi(line);
        this->initial_state.push_back(var);
    }

    getline(file, line);
    assert(line == "end_state");
}

void PlanningTask::get_goal(std::ifstream &file) {
    std::string line;
    getline(file, line);

    assert(line == "begin_goal");

    getline(file, line);
    this->n_goals = std::stoi(line);

    for (int i = 0; i < this->n_goals; i++) {
        getline(file, line);
        goal_state.push_back(parse_fact(line));
    }

    getline(file, line);
    assert(line == "end_goal");
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
    get_initial_state(file);
    get_goal(file);

    file.close();
    return 0;
}
