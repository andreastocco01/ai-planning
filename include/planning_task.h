#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

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

    bool is_used; // this flag is 1 if the action is used in the plan
    int h_cost; // the heuristic cost of the action
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

    std::vector<Action> solution;
    int solution_cost;

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

    void print_solution();
    void brute_force(int seed);
    void greedy(int seed);
    void solve(int seed);

private:
    bool goal_reached(std::vector<int> &current_state);
    void apply_axioms(std::vector<int> &current_state);
    bool check_axiom_cond(Axiom axiom, std::vector<int> &current_state);
    bool check_mutex_groups(int var_to_update, int new_value, std::vector<int> &current_state);
    int get_max_axiom_layer();
    std::vector<int> get_possible_actions_idx(std::vector<int> &current_state);
    void apply_action(Action &action, std::vector<int> &current_state);
    std::vector<int> get_min_cost_actions_idx(std::vector<int> &actions_idx);
    std::vector<int> get_min_h_cost_actions_idx(std::vector<int> &actions_idx);
    std::vector<int> get_actions_idx_having_outcome(Fact &fact);
    int h_add(std::vector<int> &current_state, Fact &fact, std::set<int> &visited);
    int compute_h_add(std::vector<int> &current_state);
    void remove_satisfied_actions(std::vector<int> &current_state, std::vector<int> &possible_actions_idx);
};

#endif
