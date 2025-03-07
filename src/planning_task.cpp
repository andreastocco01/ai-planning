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
#include <sstream>
#include <string>
#include <vector>

#include "../include/planning_task_utils.h"
#include "../include/pq.h"

#define FIND_FACT_INDEX(f)                                    \
    (std::find(this->facts.begin(), this->facts.end(), (f)) - \
     this->facts.begin())

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
    return actions_idx;
}

void PlanningTask::apply_action(int idx, std::vector<int> &current_state) {
    int count_applied_effects =
        0;  // count applied effects during this iteration
    for (int i = 0; i < this->actions[idx].n_effects; i++) {
        Effect effect = this->actions[idx].effects[i];
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
            count_applied_effects++;
        }
    }

    if (count_applied_effects > 0 &&
        this->actions[idx].applied_effects == 0) {  // first time
        IndexAction indexAction;
        indexAction.idx = idx;
        indexAction.action = this->actions[idx];
        this->solution.push_back(indexAction);
        if (this->metric == 1)
            this->solution_cost += this->actions[idx].cost;
        else
            this->solution_cost += 1;
    }

    this->actions[idx].applied_effects +=
        count_applied_effects;  // add to the total number of applied effects
                                // the effects applied during this iteration
    if (this->actions[idx].applied_effects ==
        this->actions[idx]
            .n_effects) {  // all the effects were applied in some iteration
        this->actions[idx].is_used = true;
    }
}

void PlanningTask::print_solution() {
    for (int i = 0; i < this->solution.size(); i++) {
        std::cout << this->solution[i].idx
                  << ": " + this->solution[i].action.name << std::endl;
    }
    std::cout << "Cost: " << this->solution_cost << std::endl;
}

std::vector<int> PlanningTask::get_min_h_cost_actions_idx(
    std::vector<int> &actions_idx) {
    std::vector<int> res;
    int min_cost = this->actions[actions_idx[0]].h_cost;

    // find minimum h_cost
    for (int i = 1; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].h_cost < min_cost)
            min_cost = this->actions[idx].h_cost;
    }

    // get all minimum h_cost action indexes
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].h_cost == min_cost) res.push_back(idx);
    }

    return res;
}

void PlanningTask::print_action_h_costs(std::vector<int> &actions_idx) {
    std::cout << "################################## Action h costs:"
              << std::endl;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        std::cout << this->actions[idx].name << " -> "
                  << this->actions[idx].h_cost << std::endl;
    }
}

std::vector<int> PlanningTask::get_actions_idx_having_outcome(Fact &fact) {
    std::vector<int> actions_idx;

    for (int j = 0; j < this->n_actions; j++) {
        Action action = this->actions[j];
        for (int k = 0; k < action.n_effects; k++) {
            Effect effect = action.effects[k];
            if (effect.var_affected == fact.var_idx &&
                effect.to_value == fact.var_val) {
                actions_idx.push_back(j);
            }
        }
    }

    return actions_idx;
}

// it returns also all the actions having no precondition at all
// because these actions have "all possible facts as precondition"!
std::vector<int> PlanningTask::get_actions_idx_having_precond(Fact &fact) {
    std::vector<int> actions_idx;

    for (int j = 0; j < this->n_actions; j++) {
        Action action = this->actions[j];
        for (int k = 0; k < action.n_preconds; k++) {
            Fact precond = action.preconds[k];
            if (precond == fact) {
                actions_idx.push_back(j);
            }
        }
        if (action.n_preconds == 0)
            actions_idx.push_back(
                j);  // add also the actions that doesn't have any precondition
    }

    return actions_idx;
}

int PlanningTask::h_add(std::vector<int> &current_state, Fact &fact,
                        std::set<int> &visited,
                        std::unordered_map<int, int> &cache) {
    // **Check Cache**
    if (cache.find(fact.var_idx) != cache.end()) {
        return cache[fact.var_idx];  // Return stored result
    }

    if (current_state[fact.var_idx] == fact.var_val ||
        visited.find(fact.var_idx) != visited.end())
        return 0;  // base case

    visited.insert(fact.var_idx);

    // get all the actions having "fact" as outcome
    std::vector<int> actions_idx = get_actions_idx_having_outcome(fact);

    if (actions_idx.empty())  // the fact is unreachable
        return std::numeric_limits<int>::max();

    int min_h_cost = std::numeric_limits<int>::max();

    for (int idx : actions_idx) {
        if (this->metric == 1)
            this->actions[idx].h_cost = this->actions[idx].cost;
        else
            this->actions[idx].h_cost = 1;

        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            this->actions[idx].h_cost += h_add(
                current_state, this->actions[idx].preconds[j], visited, cache);
        }

        min_h_cost = std::min(min_h_cost, this->actions[idx].h_cost);
    }

    // **Store Computed Result in Cache**
    cache[fact.var_idx] = min_h_cost;
    return min_h_cost;
}

int PlanningTask::h_max(std::vector<int> &current_state, Fact &fact,
                        std::set<int> &visited,
                        std::unordered_map<int, int> &cache) {
    // **Check Cache**
    if (cache.find(fact.var_idx) != cache.end()) {
        return cache[fact.var_idx];  // Return stored result
    }

    if (current_state[fact.var_idx] == fact.var_val ||
        visited.find(fact.var_idx) != visited.end()) {
        return 0;  // Base case
    }

    visited.insert(fact.var_idx);

    // Get all the actions having "fact" as outcome
    std::vector<int> actions_idx = get_actions_idx_having_outcome(fact);

    if (actions_idx.empty())  // The fact is unreachable
        return std::numeric_limits<int>::max();

    int min_h_cost = std::numeric_limits<int>::max();

    for (int idx : actions_idx) {
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
    cache[fact.var_idx] = min_h_cost;
    return min_h_cost;
}

int PlanningTask::h_max_optimized(std::vector<int> &current_state) {
    PriorityQueue<int> pq(
        this->map_fact_actions
            .size());  // the total number of facts is the size of the map
    int inf = std::numeric_limits<int>::max();
    std::vector<int> fact_costs(this->map_fact_actions.size(),
                                inf);  // all fact costs initialized to +inf

    // except the facts that are in the current_state
    for (int i = 0; i < current_state.size(); i++) {
        Fact f;
        f.var_idx = i;
        f.var_val = current_state[i];

        fact_costs[FIND_FACT_INDEX(f)] = 0;
        pq.push(FIND_FACT_INDEX(f), 0);  // push also these facts in the queue
    }

    while (!pq.isEmpty()) {
        Fact f = this->facts[pq.top()];
        pq.pop();
        std::vector<int> actions_idx = this->map_fact_actions[f];

        for (int i = 0; i < actions_idx.size(); i++) {
            int max_pre = 0;
            Action action = this->actions[actions_idx[i]];
            for (int j = 0; j < action.n_preconds; j++) {
                Fact pre = action.preconds[j];
                max_pre = std::max(max_pre, fact_costs[FIND_FACT_INDEX(pre)]);
            }
            // nothing to do if some preconditions are still unreachable
            if (max_pre >= inf) continue;
            int new_cost = max_pre + action.cost;
            this->actions[actions_idx[i]].h_cost = new_cost;
            for (int j = 0; j < action.n_effects; j++) {
                Fact eff;
                eff.var_idx = action.effects[j].var_affected;
                eff.var_val = action.effects[j].to_value;

                int fact_idx = FIND_FACT_INDEX(eff);

                if (new_cost < fact_costs[fact_idx]) {
                    fact_costs[fact_idx] = new_cost;
                    if (pq.has(fact_idx))
                        pq.change(fact_idx, new_cost);
                    else
                        pq.push(fact_idx, new_cost);
                }
            }
        }
    }

    int total = 0;
    for (int i = 0; i < this->n_goals; i++) {
        total = std::max(total, fact_costs[FIND_FACT_INDEX(goal_state[i])]);
    }
    return total;
}

void PlanningTask::create_structs() {
    for (int i = 0; i < this->n_actions; i++) {
        Action action = this->actions[i];
        std::vector<Fact> preconds = action.preconds;
        std::vector<Effect> effects = action.effects;

        // add precond facts
        for (int j = 0; j < preconds.size(); j++) {
            if (this->map_fact_actions.find(preconds[j]) ==
                this->map_fact_actions.end()) {  // inserting new entry
                this->map_fact_actions[preconds[j]] =
                    get_actions_idx_having_precond(preconds[j]);
                this->facts.push_back(preconds[j]);
            }
        }

        // add effect facts
        for (int j = 0; j < effects.size(); j++) {
            Fact f;
            f.var_idx = effects[j].var_affected;
            f.var_val = effects[j].to_value;

            if (this->map_fact_actions.find(f) ==
                this->map_fact_actions.end()) {  // inserting new entry
                this->map_fact_actions[f] = get_actions_idx_having_precond(f);
                this->facts.push_back(f);
            }
        }
    }
    // add also facts that are in the initial state, but not in any action
    // precond!
    for (int i = 0; i < this->initial_state.size(); i++) {
        Fact f;
        f.var_idx = i;
        f.var_val = this->initial_state[i];
        if (this->map_fact_actions.find(f) == this->map_fact_actions.end()) {
            this->map_fact_actions[f] = get_actions_idx_having_precond(f);
            this->facts.push_back(f);
        }
    }
}

int PlanningTask::compute_heuristic(std::vector<int> &current_state,
                                    int heuristic) {
    int total = 0;
    std::unordered_map<int, int> cache;  // Cache to store heuristic values

    if (heuristic == 2) {
        for (int i = 0; i < this->n_goals; i++) {
            std::set<int> visited;
            total += h_add(current_state, this->goal_state[i], visited, cache);
        }
    } else if (heuristic == 3) {
        for (int i = 0; i < this->n_goals; i++) {
            std::set<int> visited;
            total = std::max(total, h_max(current_state, this->goal_state[i],
                                          visited, cache));
        }
    }

    return total;
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

    if (heuristic == 4) {
        std::cout << "Creating structs..." << std::endl;
        create_structs();
        std::cout << "Done" << std::endl;
    }

    bool no_solution = false;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);

        // calculate heuristic costs
        if (heuristic == 2 || heuristic == 3) {
            int total = compute_heuristic(current_state, heuristic);
            if (total < estimated_cost) {
                estimated_cost = total;
                std::cout << "New estimated cost to reach the goal state: "
                          << estimated_cost << std::endl;
            }
        }

        if (heuristic == 4) {
            int total = h_max_optimized(current_state);
            if (total < estimated_cost) {
                estimated_cost = total;
                std::cout << "New estimated cost to reach the goal state: "
                          << estimated_cost << std::endl;
            }
        }

        // get possible actions
        std::vector<int> possible_actions_idx =
            get_possible_actions_idx(current_state, true);

        // remove actions having outcome already satisfied
        remove_satisfied_actions(current_state, possible_actions_idx);

        if (possible_actions_idx.empty()) {
            no_solution = true;
            break;
        }

        // get min h cost actions
        int action_to_apply_idx;
        if (heuristic == 0) {  // random
            action_to_apply_idx =
                possible_actions_idx[PlanningTaskUtils::get_random_number(
                    0, possible_actions_idx.size())];
        } else {
            std::vector<int> min_h_cost_actions_idx =
                get_min_h_cost_actions_idx(possible_actions_idx);
            action_to_apply_idx =
                min_h_cost_actions_idx[PlanningTaskUtils::get_random_number(
                    0, min_h_cost_actions_idx.size())];
        }

        // apply action
        apply_action(action_to_apply_idx, current_state);
    }

    if (time_limit != -1) {
        kill(pid, SIGTERM);
    }

    if (no_solution) {
        std::cout << "Solution does not exist!" << std::endl;
        return -1;
    } else {
        std::cout << "Solution found!" << std::endl;
    }

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
    }
    return true;
}
