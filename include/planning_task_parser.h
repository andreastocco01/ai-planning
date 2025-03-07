#ifndef PLANNING_TASK_PARSER_H
#define PLANNING_TASK_PARSER_H

#include <vector>

#include "planning_task.h"

class PlanningTaskParser {
   public:
    PlanningTask parse_from_file(std::string filenamme);

   private:
    void assert_version(std::ifstream &file);
    int get_metric(std::ifstream &file);
    std::vector<Variable> get_variables(std::ifstream &file);
    Fact parse_fact(std::string line);
    std::vector<MutexGroup> get_facts(std::ifstream &file);
    std::vector<int> get_initial_state(std::ifstream &file, int n_vars);
    std::vector<Fact> get_goal(std::ifstream &file);
    std::vector<Action> get_actions(std::ifstream &file);
    std::vector<Axiom> get_axioms(std::ifstream &file);
};

#endif
