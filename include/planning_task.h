#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <string>
#include <vector>

class Variable {
public:
    std::string name;
    int axiom_layer;
    int range;
    std::vector<std::string> sym_names;
};

class Fact {
public:
    int var_idx;
    int var_val;
};

class MutexGroup {
public:
    int n_facts;
    std::vector<Fact> facts;
};

class PlanningTask {
public:
    int parse_from_file(std::string filenamme);
    void print();
private:
    int metric; // 0 no action costs, 1 action costs
    int n_vars;
    std::vector<Variable> vars;
    int n_mutex;
    std::vector<MutexGroup> mutexes;
    std::vector<int> initial_state;

    void assert_version(std::ifstream &file);
    void get_metric(std::ifstream &file);
    void get_variables(std::ifstream &file);
    void get_facts(std::ifstream &file);
    void get_initial_state(std::ifstream &file);

    void print_vars();
    void print_facts();
    void print_initial_state();
};

#endif
