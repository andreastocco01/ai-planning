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

class PlanningTask {
public:
    int parse_from_file(std::string filenamme);
    void print_vars();
private:
    int metric; // 0 no action costs, 1 action costs
    std::vector<Variable> vars;

    void assert_version(std::ifstream &file);
    void get_metric(std::ifstream &file);
    void get_variables(std::ifstream &file);
};

#endif
