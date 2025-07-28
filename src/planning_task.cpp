#include "../include/planning_task.h"

#include <signal.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <ostream>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

#include "../include/planning_task_utils.h"
#include "../include/pq.h"

#define FIND_FACT_INDEX(f) (this->fact_to_index[f])

PlanningTask::PlanningTask(int metric, int n_vars, std::vector<Variable> &vars,
                           int n_mutex, std::vector<MutexGroup> &mutexes,
                           std::vector<int> &initial_state, int n_goals,
                           std::vector<Fact> &goal_state, int n_actions,
                           std::vector<Action> &actions, int n_axioms,
                           std::vector<Axiom> &axioms) {
    this->metric = metric;
    this->n_vars = n_vars;
    this->vars = vars, this->n_mutex = n_mutex;
    this->mutexes = mutexes;
    this->initial_state = initial_state;
    this->n_goals = n_goals;
    this->goal_state = goal_state;
    this->n_actions = n_actions;
    this->actions = actions;
    this->n_axioms = n_axioms;
    this->axioms = axioms;

    this->solution_cost = 0;
}

PlanningTask::PlanningTask(const PlanningTask &other) {
    this->metric = other.metric;
    this->n_vars = other.n_vars;
    this->vars = other.vars, this->n_mutex = other.n_mutex;
    this->mutexes = other.mutexes;
    this->n_actions = other.n_actions;
    this->actions = other.actions;
    reset_actions_metadata();
    for (int i = 0; i < this->n_actions; i++) {
        this->actions[i].is_used = false;
    }
    this->n_axioms = other.n_axioms;
    this->axioms = other.axioms;

    this->solution_cost = 0;
}

/*
    check if the current state is a goal state
*/
bool PlanningTask::goal_reached(std::vector<int> &current_state) {
    for (int i = 0; i < this->n_goals; i++) {
        int idx = this->goal_state[i].var_idx;
        if (this->goal_state[i].var_val != current_state[idx]) return false;
    }
    return true;
}

/*
    check if the application of an axiom would break a mutex
    in each mutex at most one fact can be true

    if a mutex already have a true fact, then we cannot apply any update to that
   mutex
*/
bool PlanningTask::check_mutex_groups(int var_to_update, int new_value,
                                      std::vector<int> &current_state) {
    for (int i = 0; i < this->n_mutex; i++) {
        MutexGroup mutex = this->mutexes[i];
        bool mutex_fact_in_solution = false;
        bool update_in_mutex = false;
        for (int j = 0; j < mutex.n_facts; j++) {
            int fact_var = mutex.facts[j].var_idx;
            int fact_value = mutex.facts[j].var_val;
            if (current_state[fact_var] == fact_value)
                mutex_fact_in_solution = true;
            if (fact_var == var_to_update && fact_value == new_value)
                update_in_mutex = true;
        }
        if (mutex_fact_in_solution && update_in_mutex) return false;
    }
    return true;
}

int PlanningTask::get_max_axiom_layer() {
    int max = -1;
    for (int i = 0; i < this->n_vars; i++)
        if (this->vars[i].axiom_layer > max) max = this->vars[i].axiom_layer;
    return max;
}

bool PlanningTask::check_axiom_cond(Axiom axiom,
                                    std::vector<int> &current_state) {
    for (int i = 0; i < axiom.n_conds; i++) {
        if (current_state[axiom.conds[i].var_idx] != axiom.conds[i].var_val)
            return false;
    }
    return true;
}

void PlanningTask::apply_axioms(std::vector<int> &current_state) {
    int max_axiom_layer = get_max_axiom_layer();
    for (int axiom_layer = 0; axiom_layer <= max_axiom_layer; axiom_layer++) {
        for (int i = 0; i < this->n_axioms; i++) {
            Axiom axiom = this->axioms[i];
            if (this->vars[axiom.affected_var].axiom_layer == axiom_layer &&
                check_axiom_cond(axiom, current_state)) {
                if ((current_state[axiom.affected_var] == axiom.from_value ||
                     axiom.from_value == -1) &&
                    check_mutex_groups(axiom.affected_var, axiom.to_value,
                                       current_state)) {
                    current_state[axiom.affected_var] = axiom.to_value;
                }
            }
        }
    }
}

std::vector<int> PlanningTask::get_possible_actions_idx(
    std::vector<int> &current_state, bool check_usage) {
    std::vector<int> actions_idx;
    for (int i = 0; i < this->n_actions; i++) {
        Action action = this->actions[i];
        if (check_usage && action.is_used)
            continue;  // skip actions already used
        int j;
        for (j = 0; j < action.n_preconds; j++)
            if (current_state[action.preconds[j].var_idx] !=
                action.preconds[j].var_val)
                break;
        if (j == action.n_preconds) {
            actions_idx.push_back(i);
        }
    }
    std::stable_sort(
        actions_idx.begin(), actions_idx.end(), [this](int idx_a, int idx_b) {
            return this->actions[idx_a].h_cost < this->actions[idx_b].h_cost;
        });
    return actions_idx;
}

int PlanningTask::compute_next_state(int idx, std::vector<int> &current_state) {
    int n_applied_effects = 0;  // count applied effects during this iteration
    for (int i = 0; i < this->actions[idx].n_effects; i++) {
        Effect effect = this->actions[idx].effects[i];
        int j;
        for (j = 0; j < effect.n_effect_conds; j++) {
            Fact effect_cond = effect.effect_conds[j];
            if (current_state[effect_cond.var_idx] != effect_cond.var_val &&
                effect_cond.var_val != -1)
                break;
        }
        if (j < effect.n_effect_conds) {  // the effect cannot be applied
            this->pending_effects.push_back(effect);
            continue;
        }
        int var = effect.var_affected;
        if ((current_state[var] == effect.from_value ||
             effect.from_value == -1) &&
            check_mutex_groups(var, effect.to_value, current_state)) {
            current_state[var] = effect.to_value;
            n_applied_effects++;
        } else {
            this->pending_effects.push_back(effect);
        }
    }
    return n_applied_effects;
}

int PlanningTask::apply_action(int idx, std::vector<int> &current_state) {
    int n_applied_effects = compute_next_state(idx, current_state);
    if (n_applied_effects) {  // at least one effect was
                              // applied
        IndexAction indexAction;
        indexAction.idx = idx;
        indexAction.action = this->actions[idx];
        this->solution.push_back(indexAction);
        if (this->metric == 1)
            this->solution_cost += this->actions[idx].cost;
        else
            this->solution_cost += 1;
        this->actions[idx].is_used = true;
    }
    return n_applied_effects;
}

void PlanningTask::print_solution() {
    for (int i = 0; i < this->solution.size(); i++) {
        std::cout << this->solution[i].idx << ": "
                  << this->solution[i].action.name << std::endl;
    }
    std::cout << "Cost: " << this->solution_cost << std::endl;
}

void PlanningTask::print_action_h_costs(std::vector<int> &actions_idx) {
    std::cout << "################################## H COSTS" << std::endl;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        std::cout << this->actions[idx].h_cost << " ";
    }
    std::cout << std::endl;
}

void PlanningTask::create_structs() {
    std::unordered_set<Fact, FactHasher> unique_facts;

    // Add facts from INITIAL STATE
    for (int var = 0; var < initial_state.size(); var++) {
        Fact f{var, initial_state[var]};
        unique_facts.insert(f);
    }

    // Add facts from GOALS
    for (const Fact &goal : goal_state) {
        unique_facts.insert(goal);
    }

    for (int i = 0; i < this->n_actions; i++) {
        Action &action = this->actions[i];
        if (action.n_preconds == 0) this->actions_no_preconds.push_back(i);

        // Add preconditions to map_fact_actions
        for (auto &precond : action.preconds) {
            this->map_precond_actions[precond].push_back(i);
            unique_facts.insert(precond);
        }

        // Add effects for ALL actions
        for (const Effect &eff : action.effects) {
            Fact f{eff.var_affected, eff.to_value};
            this->map_effect_actions[f].push_back(i);
            unique_facts.insert(f);
        }
    }

    // add facts from axioms
    for (int i = 0; i < this->n_axioms; i++) {
        Fact f{this->axioms[i].affected_var, this->axioms[i].to_value};
        unique_facts.insert(f);
    }

    // Convert the set to a vector for indexing
    this->facts.assign(unique_facts.begin(), unique_facts.end());

    // Precompute fact_to_index map
    this->fact_to_index.clear();
    for (size_t i = 0; i < this->facts.size(); i++) {
        this->fact_to_index[this->facts[i]] = i;
    }
}

void PlanningTask::remove_satisfied_actions(
    std::vector<int> &current_state, std::vector<int> &possible_actions_idx) {
    for (int i = possible_actions_idx.size() - 1; i >= 0; i--) {
        int idx = possible_actions_idx[i];
        std::vector<Effect> effects = this->actions[idx].effects;
        int count = 0;
        for (int j = 0; j < effects.size(); j++) {
            if (current_state[effects[j].var_affected] == effects[j].to_value)
                count++;
        }
        if (count == effects.size()) {
            possible_actions_idx.erase(possible_actions_idx.begin() + i);
            this->actions[idx].is_used =
                true;  // this action shouldn't be returned anymore
        }
    }
}

int PlanningTask::h_max(std::vector<int> &current_state, Fact &fact,
                        std::unordered_set<int> &visited,
                        std::unordered_map<int, int> &cache) {
    int fact_idx = FIND_FACT_INDEX(fact);

    // **Check Cache**
    if (cache.find(fact_idx) != cache.end()) {
        return cache[fact_idx];  // Return stored result
    }

    if (current_state[fact.var_idx] == fact.var_val ||
        visited.find(fact_idx) != visited.end()) {
        return 0;  // Base case
    }

    visited.insert(fact_idx);

    // Get all the actions having "fact" as outcome
    std::vector<int> actions_idx = this->map_effect_actions[fact];

    if (actions_idx.empty())  // The fact is unreachable
        return std::numeric_limits<int>::max();

    int min_h_cost = std::numeric_limits<int>::max();

    for (int idx : actions_idx) {
        if (this->actions[idx].is_used) continue;
        if (this->metric == 1)
            this->actions[idx].h_cost = this->actions[idx].cost;
        else
            this->actions[idx].h_cost = 1;

        int max_cost = 0;
        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            max_cost = std::max(
                max_cost, h_max(current_state, this->actions[idx].preconds[j],
                                visited, cache));
        }

        this->actions[idx].h_cost += max_cost;
        min_h_cost = std::min(min_h_cost, this->actions[idx].h_cost);
    }

    // **Store Computed Result in Cache**
    cache[fact_idx] = min_h_cost;
    return min_h_cost;
}

void PlanningTask::reset_actions_metadata() {
    for (int i = 0; i < this->n_actions; i++) {
        this->actions[i].h_cost = std::numeric_limits<int>::max();
    }
}

int PlanningTask::compute_heuristic(std::vector<int> &current_state,
                                    int heuristic) {
    int total = 0;
    std::unordered_map<int, int> cache;  // Cache to store heuristic values

    if (heuristic == 2 || heuristic == 3) {
        for (int i = 0; i < this->n_goals; i++) {
            std::unordered_set<int> visited;
            total = std::max(total, h_max(current_state, this->goal_state[i],
                                          visited, cache));
        }
    }

    return total;
}

void PlanningTask::backward_cost_propagation(std::vector<int> &current_state,
                                             int heuristic) {
    PriorityQueue<int> pq(this->facts.size());
    int inf = std::numeric_limits<int>::max();
    std::vector<int> fact_costs(this->facts.size(), inf);

    // Initialize goal state facts
    for (int i = 0; i < this->goal_state.size(); i++) {
        int idx = FIND_FACT_INDEX(this->goal_state[i]);
        fact_costs[idx] = 0;
        pq.push(idx, 0);
    }

    while (!pq.isEmpty()) {
        int fact_idx = pq.top();
        pq.pop();
        Fact f = this->facts[fact_idx];

        if (current_state[f.var_idx] == f.var_val) continue;
        std::vector<int> actions = this->map_effect_actions[f];

        for (int i = 0; i < actions.size(); i++) {
            Action &current_action = this->actions[actions[i]];

            if (current_action.is_used) continue;
            int new_cost;
            if (heuristic == 4) {
                new_cost = (this->metric == 1)
                               ? current_action.cost + fact_costs[fact_idx]
                               : 1 + fact_costs[fact_idx];

                if (new_cost >= current_action.h_cost) continue;
            } else if (heuristic == 5) {
                int max_cost = fact_costs[fact_idx];
                for (int j = 0; j < current_action.n_effects; j++) {
                    Fact eff{current_action.effects[j].var_affected,
                             current_action.effects[j].to_value};
                    if (fact_costs[FIND_FACT_INDEX(eff)] != inf)
                        max_cost = std::max(max_cost,
                                            fact_costs[FIND_FACT_INDEX(eff)]);
                }
                new_cost = this->metric == 1 ? current_action.cost + max_cost
                                             : 1 + max_cost;
            } else if (heuristic == 6) {
                int sum = 0;
                for (int j = 0; j < current_action.n_effects; j++) {
                    Fact eff{current_action.effects[j].var_affected,
                             current_action.effects[j].to_value};
                    if (fact_costs[FIND_FACT_INDEX(eff)] != inf)
                        sum += fact_costs[FIND_FACT_INDEX(eff)];
                }
                new_cost =
                    this->metric == 1 ? current_action.cost + sum : 1 + sum;
            }

            current_action.h_cost = new_cost;
            for (int j = 0; j < current_action.n_preconds; j++) {
                Fact pre = current_action.preconds[j];
                int pre_idx = FIND_FACT_INDEX(pre);
                if (new_cost < fact_costs[pre_idx]) {
                    fact_costs[pre_idx] = new_cost;
                    if (pq.has(pre_idx))
                        pq.change(pre_idx, new_cost);
                    else
                        pq.push(pre_idx, new_cost);
                }
            }
        }
    }
}

int PlanningTask::apply_pending_effects(std::vector<int> &current_state) {
    int n_applied_effects = 0;
    for (int i = this->pending_effects.size() - 1; i >= 0; i--) {
        Effect effect = this->pending_effects[i];
        int j;
        for (j = 0; j < effect.n_effect_conds; j++) {
            Fact effect_cond = effect.effect_conds[j];
            if (current_state[effect_cond.var_idx] != effect_cond.var_val &&
                effect_cond.var_val != -1)
                break;
        }
        if (j < effect.n_effect_conds)  // the effect cannot be applied
            continue;
        int var = effect.var_affected;
        if ((current_state[var] == effect.from_value ||
             effect.from_value == -1) &&
            check_mutex_groups(var, effect.to_value, current_state)) {
            current_state[var] = effect.to_value;
            this->pending_effects.erase(this->pending_effects.begin() + i);
            n_applied_effects++;
        }
    }
    return n_applied_effects;
}

// apply each possible action
// re-compute hmax
// add to h_cost the result of hmax
void PlanningTask::look_ahead(std::vector<int> &current_state,
                              std::vector<int> &possible_actions_idx,
                              int heuristic) {
    std::vector<int> costs;
    for (int i = 0; i < possible_actions_idx.size(); i++)
        costs.push_back(this->actions[possible_actions_idx[i]].h_cost);

    // simulate action application
    for (int k = 0; k < possible_actions_idx.size(); k++) {
        std::vector<int> new_state = current_state;
        int idx = possible_actions_idx[k];
        for (int i = 0; i < this->actions[idx].n_effects; i++) {
            Effect effect = this->actions[idx].effects[i];
            int j;
            for (j = 0; j < effect.n_effect_conds; j++) {
                Fact effect_cond = effect.effect_conds[j];
                if (new_state[effect_cond.var_idx] != effect_cond.var_val &&
                    effect_cond.var_val != -1)
                    break;
            }
            if (j < effect.n_effect_conds)  // the effect cannot be applied
                continue;
            int var = effect.var_affected;
            if ((new_state[var] == effect.from_value ||
                 effect.from_value == -1) &&
                check_mutex_groups(var, effect.to_value, new_state)) {
                new_state[var] = effect.to_value;
            }
        }

        reset_actions_metadata();
        int total = compute_heuristic(new_state, heuristic);
        costs[k] = total + costs[k] < 0 ? std::numeric_limits<int>::max()
                                        : total + costs[k];
    }

    for (int i = 0; i < possible_actions_idx.size(); i++)
        this->actions[possible_actions_idx[i]].h_cost = costs[i];
    possible_actions_idx =
        get_possible_actions_idx(current_state, true);  // get sorted actions
}

int PlanningTask::solve(int seed, int heuristic, bool debug, int time_limit) {
    int pid;
    if (time_limit != -1) {
        pid = fork();
        if (pid == 0) {  // child process
            sleep(time_limit);
            std::cerr << "Time limit reached. Killing parent process."
                      << std::endl;
            kill(getppid(), SIGTERM);  // Send SIGTERM to the parent process
            exit(0);                   // Exit child process
        }
    }

    // parent process
    srand(seed);
    std::vector<int> current_state = this->initial_state;
    int estimated_cost = std::numeric_limits<int>::max();

    // h_cost = cost in greedy
    if (heuristic == 1) {
        for (int i = 0; i < this->n_actions; i++) {
            if (this->metric == 1)
                this->actions[i].h_cost = this->actions[i].cost;
            else
                this->actions[i].h_cost = 1;  // greedy becomes random
        }
    }

    std::cout << "Creating structs...";
    create_structs();
    std::cout << "Done" << std::endl;

    bool no_solution = false;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);
        if (int n = apply_pending_effects(current_state))
            std::cout << "Applied " << n << " pending effects" << std::endl;

        // calculate heuristic costs
        if (heuristic == 2 || heuristic == 3) {
            reset_actions_metadata();
            int total = compute_heuristic(current_state, heuristic);
            if (total < estimated_cost) {
                estimated_cost = total;
                std::cout << "New estimated cost: " << estimated_cost
                          << std::endl;
            }
        }

        if (heuristic == 4 || heuristic == 5 || heuristic == 6) {
            reset_actions_metadata();
            backward_cost_propagation(current_state, heuristic);
        }

        // get possible actions
        std::vector<int> possible_actions_idx =
            get_possible_actions_idx(current_state, true);

        // if the first action has infinite cost, the problem is
        // infeasible (beacuse possible_actions_idx is sorted)
        if (possible_actions_idx.empty() ||
            (heuristic > 0 && this->actions[possible_actions_idx[0]].h_cost ==
                                  std::numeric_limits<int>::max())) {
            no_solution = true;
            break;
        }

        /*std::cout << "POSSIBLE ACTIONS: ";
        PlanningTaskUtils::print_planning_task_state(possible_actions_idx);
        print_action_h_costs(possible_actions_idx);*/

        // remove actions having outcome already satisfied
        remove_satisfied_actions(current_state, possible_actions_idx);

        if (possible_actions_idx.empty()) {
            no_solution = true;
            break;
        }

        if (heuristic == 3) {
            look_ahead(current_state, possible_actions_idx, heuristic);
        }

        int action_to_apply_idx;
        int n_applied_effects = 0;

        while (!n_applied_effects) {
            int idx;
            if (heuristic == 0) {
                idx = PlanningTaskUtils::get_random_number(
                    0, possible_actions_idx.size());
            } else {
                int i = 0;
                int min_cost = this->actions[possible_actions_idx[0]].h_cost;
                while (i < possible_actions_idx.size() &&
                       this->actions[possible_actions_idx[i]].h_cost ==
                           min_cost)
                    i++;
                idx = PlanningTaskUtils::get_random_number(0, i);
            }

            action_to_apply_idx = possible_actions_idx[idx];
            n_applied_effects =
                apply_action(action_to_apply_idx, current_state);
            possible_actions_idx.erase(possible_actions_idx.begin() + idx);
        }

        // std::cout << "APPLIED ACTION: " << action_to_apply_idx << std::endl;
    }

    if (time_limit != -1) {
        kill(pid, SIGTERM);
    }

    if (no_solution) return -1;

    if (debug) {
        if (check_integrity())
            std::cout << "Integrity check passed!" << std::endl;
        else
            std::cout << "Integrity check NOT passed!" << std::endl;
    }

    return 0;
}

bool PlanningTask::check_integrity() {
    std::vector<int> current_state = this->initial_state;
    int cost = 0;
    for (int k = 0; k < this->solution.size(); k++) {
        IndexAction indexAction = this->solution[k];
        std::vector<int> actions_idx =
            get_possible_actions_idx(current_state, false);
        int p = 0;
        for (; p < actions_idx.size(); p++) {
            if (actions_idx[p] == indexAction.idx) break;
        }
        if (p == actions_idx.size())
            return false;  // action not applicable at this point
        for (int i = 0; i < indexAction.action.n_effects; i++) {
            Effect effect = indexAction.action.effects[i];
            int j;
            for (j = 0; j < effect.n_effect_conds; j++) {
                std::vector<Fact> effect_conds = effect.effect_conds;
                if (current_state[effect_conds[j].var_idx] !=
                        effect_conds[j].var_val &&
                    effect_conds[j].var_val != -1)
                    break;
            }
            if (j < effect.n_effect_conds)  // the effect cannot be applied
                continue;
            int var = effect.var_affected;
            if ((current_state[var] == effect.from_value ||
                 effect.from_value == -1) &&
                check_mutex_groups(var, effect.to_value, current_state)) {
                current_state[var] = effect.to_value;
            }
        }
        cost += this->actions[indexAction.idx].cost;
    }
    if (cost == this->solution_cost) return true;
    return false;
}

std::string encode(const std::vector<int> &state) {
    std::string key;
    for (int v : state) {
        key += std::to_string(v) + ",";
    }
    return key;
}

int PlanningTask::dfs(int max_depth) {
    std::stack<DfsNode> stack;
    std::vector<int> best_path;
    int best_path_cost = std::numeric_limits<int>::max();
    std::set<std::string> visited;

    for (int action_idx : get_possible_actions_idx(this->initial_state, true)) {
        std::vector<int> new_state = this->initial_state;
        compute_next_state(action_idx, new_state);
        stack.push({action_idx,
                    new_state,
                    {action_idx},
                    (this->metric == 1) ? this->actions[action_idx].cost : 1});
    }

    while (!stack.empty()) {
        DfsNode node = stack.top();
        stack.pop();

        if (node.path.size() > max_depth) continue;

        if (goal_reached(node.state)) {
            if (node.cost < best_path_cost) {
                best_path_cost = node.cost;
                best_path = node.path;
            }
            continue;
        }

        std::string key = encode(node.state);
        if (visited.count(key)) continue;
        visited.insert(key);

        std::vector<int> successors =
            get_possible_actions_idx(node.state, true);
        for (int action_idx : successors) {
            std::vector<int> new_state = node.state;
            compute_next_state(action_idx, new_state);

            std::vector<int> new_path = node.path;
            new_path.push_back(action_idx);

            int new_cost =
                node.cost +
                ((this->metric == 1) ? this->actions[action_idx].cost : 1);

            stack.push({action_idx, new_state, new_path, new_cost});
        }
    }

    this->solution_cost = best_path_cost;
    for (int idx : best_path) {
        this->solution.push_back({idx, this->actions[idx]});
    }

    return 0;
}
