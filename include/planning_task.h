#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <sstream>
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

class Effect {
public:
    int n_effect_conds;
    std::vector<Fact> effect_conds;
    int var_affected;
    int from_value;
    int to_value;
};

class Action {
public:
    std::string name;
    int n_preconds;
    std::vector<Fact> preconds;
    int n_effects;
    std::vector<Effect> effects;
    int cost;
};

class Axiom {
public:
    int n_conds;
    std::vector<Fact> conds;
    int affected_var;
    int from_value;
    int to_value;
};

class PlanningTask {
public:
    int metric; // 0 no action costs, 1 action costs
    int n_vars;
    std::vector<Variable> vars;
    int n_mutex;
    std::vector<MutexGroup> mutexes;
    std::vector<int> initial_state;
    int n_goals;
    std::vector<Fact> goal_state;
    int n_actions;
    std::vector<Action> actions;
    int n_axioms;
    std::vector<Axiom> axioms;

    std::vector<Action> best_solution;
    int best_solution_cost;

    PlanningTask(int metric,
        int n_vars,
        std::vector<Variable> &vars,
        int n_mutex,
        std::vector<MutexGroup> &mutexes,
        std::vector<int> &initial_state,
        int n_goals,
        std::vector<Fact> &goal_state,
        int n_actions,
        std::vector<Action> &actions,
        int n_axioms,
        std::vector<Axiom> &axioms);

    void greedy();

private:
    bool goal_reached(std::vector<int> &current_state);
    void apply_axioms(std::vector<int> &current_state);
    bool check_axiom_cond(Axiom axiom, std::vector<int> &current_state);
    bool check_mutex_groups(int var_to_update, int new_value, std::vector<int> &current_state);
    int get_max_axiom_layer();
};

#endif
