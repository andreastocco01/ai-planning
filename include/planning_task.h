#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

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
    int hash() const {
        return std::hash<int>{}(var_idx) ^ (std::hash<int>{}(var_val) << 1);
    }

    // Define equality operator
    bool operator==(const Fact& other) const {
        return var_idx == other.var_idx && var_val == other.var_val;
    }
};

// Custom hash function
struct FactHasher {
    int operator()(const Fact& f) const {
        return f.hash();
    }
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
    int applied_effects;
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

class IndexAction {
public:
    int idx;
    Action action;
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
    std::unordered_map<Fact, std::vector<int>, FactHasher> map_fact_actions;
    std::vector<Fact> facts; // mapping index -> Fact

    std::vector<IndexAction> solution;
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
    int solve(int seed, int heuristic, bool debug, int time_limit);

private:
    bool goal_reached(std::vector<int> &current_state);
    void apply_axioms(std::vector<int> &current_state);
    bool check_axiom_cond(Axiom axiom, std::vector<int> &current_state);
    bool check_mutex_groups(int var_to_update, int new_value, std::vector<int> &current_state);
    int get_max_axiom_layer();
    std::vector<int> get_possible_actions_idx(std::vector<int> &current_state, bool check_usage);
    void apply_action(int idx, std::vector<int> &current_state);
    std::vector<int> get_min_h_cost_actions_idx(std::vector<int> &actions_idx);
    std::vector<int> get_actions_idx_having_outcome(Fact &fact);
    std::vector<int> get_actions_idx_having_precond(Fact &fact);
    int h_add(std::vector<int> &current_state, Fact &fact, std::set<int> &visited, std::unordered_map<int, int> &cache);
    int h_max(std::vector<int> &current_state, Fact &fact, std::set<int> &visited, std::unordered_map<int, int> &cache);
    int h_max_optimized(std::vector<int> &current_state);
    int compute_heuristic(std::vector<int> &current_state, int heuristic);
    void remove_satisfied_actions(std::vector<int> &current_state, std::vector<int> &possible_actions_idx);
    void print_action_h_costs(std::vector<int> &actions_idx);
    bool check_integrity();
    void create_structs();
};

#endif
