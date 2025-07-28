#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    int hash() const {
        return std::hash<int>{}(var_idx) ^ (std::hash<int>{}(var_val) << 1);
    }

    // Define equality operator
    bool operator==(const Fact &other) const {
        return var_idx == other.var_idx && var_val == other.var_val;
    }

    bool operator!=(const Fact &other) const {
        return !(this->var_idx == other.var_idx &&
                 this->var_val == other.var_val);
    }
};

// Custom hash function
struct FactHasher {
    int operator()(const Fact &f) const { return f.hash(); }
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

    bool is_used;  // this flag is 1 if the action is used in the plan
    int h_cost;    // the heuristic cost of the action
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

class DfsNode {
   public:
    int action_idx;
    std::vector<int> state;
    std::vector<int> path;
    int cost;
};

class PlanningTask {
   public:
    int metric;  // 0 no action costs, 1 action costs
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

    std::vector<IndexAction> solution;
    int solution_cost;
    std::vector<Effect> pending_effects;

    PlanningTask() {}

    PlanningTask(int metric, int n_vars, std::vector<Variable> &vars,
                 int n_mutex, std::vector<MutexGroup> &mutexes,
                 std::vector<int> &initial_state, int n_goals,
                 std::vector<Fact> &goal_state, int n_actions,
                 std::vector<Action> &actions, int n_axioms,
                 std::vector<Axiom> &axioms);

    // Copy constructor
    PlanningTask(const PlanningTask &other);

    void print_solution();
    bool check_integrity();
    int solve(int seed, int heuristic, bool debug, int time_limit);
    int dfs(int max_depth);

   private:
    std::unordered_map<Fact, std::vector<int>, FactHasher> map_precond_actions;
    std::unordered_map<Fact, std::vector<int>, FactHasher> map_effect_actions;
    std::vector<Fact> facts;  // mapping index -> Fact
    std::unordered_map<Fact, int, FactHasher> fact_to_index;
    std::vector<int> actions_no_preconds;

    bool goal_reached(std::vector<int> &current_state);
    void apply_axioms(std::vector<int> &current_state);
    bool check_axiom_cond(Axiom axiom, std::vector<int> &current_state);
    bool check_mutex_groups(int var_to_update, int new_value,
                            std::vector<int> &current_state);
    int get_max_axiom_layer();
    std::vector<int> get_possible_actions_idx(std::vector<int> &current_state,
                                              bool check_usage);
    int apply_action(int idx, std::vector<int> &current_state);
    int h_max(std::vector<int> &current_state, Fact &fact,
              std::unordered_set<int> &visited,
              std::unordered_map<int, int> &cache);
    int compute_heuristic(std::vector<int> &current_state, int heuristic);
    void remove_satisfied_actions(std::vector<int> &current_state,
                                  std::vector<int> &possible_actions_idx);
    void print_action_h_costs(std::vector<int> &actions_idx);
    void create_structs();
    void reset_actions_metadata();
    void backward_cost_propagation(std::vector<int> &current_state,
                                   int heuristic);
    int apply_pending_effects(std::vector<int> &current_state);
    void look_ahead(std::vector<int> &current_state,
                    std::vector<int> &possible_actions_idx, int heuristic);
    int compute_next_state(int idx, std::vector<int> &current_state);
};

#endif
