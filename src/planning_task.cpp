#include "../include/planning_task.h"
#include "../include/planning_task_utils.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cassert>
#include <iterator>
#include <limits>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

PlanningTask::PlanningTask(int metric,
    int n_vars,
    std::vector<Variable> &vars,
    int n_mutex,
    std::vector<MutexGroup> &mutexes,
    std::vector<int> &initial_state,
    int n_goals,
    std::vector<Fact> &goal_state,
    int n_actions,
    std::vector<Action> &actions,
    int n_axioms, std::vector<Axiom> &axioms) {
        this->metric = metric;
        this->n_vars = n_vars;
        this->vars = vars,
        this->n_mutex = n_mutex;
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
        if (this->goal_state[i].var_val != current_state[idx])
            return false;
    }
    return true;
}

/*
    check if the application of an axiom would break a mutex
    in each mutex at most one fact can be true

    if a mutex already have a true fact, then we cannot apply any update to that mutex
*/
bool PlanningTask::check_mutex_groups(int var_to_update, int new_value, std::vector<int> &current_state) {
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
        if (mutex_fact_in_solution && update_in_mutex)
            return false;
    }
    return true;
}

int PlanningTask::get_max_axiom_layer(){
    int max = -1;
    for (int i = 0; i < this->n_vars; i++)
        if (this->vars[i].axiom_layer > max)
            max = this->vars[i].axiom_layer;
    return max;
}

bool PlanningTask::check_axiom_cond(Axiom axiom, std::vector<int> &current_state) {
    for (int i = 0; i < axiom.n_conds; i++) {
        if (current_state[axiom.conds[i].var_idx] != axiom.conds[i].var_val)
            return false;
    }
    return true;
}

void PlanningTask::apply_axioms(std::vector<int> &current_state) {
    int max_axiom_layer = get_max_axiom_layer();
    for (int axiom_layer = 0; axiom_layer <= max_axiom_layer; axiom_layer++) {
        for (int i = 0; i < this->n_axioms; i++){
            Axiom axiom = this->axioms[i];
            if (this->vars[axiom.affected_var].axiom_layer == axiom_layer && check_axiom_cond(axiom, current_state)) {
                if ((current_state[axiom.affected_var] == axiom.from_value || axiom.from_value == -1) &&
                    check_mutex_groups(axiom.affected_var, axiom.to_value, current_state)) {
                    current_state[axiom.affected_var] = axiom.to_value;
                }
            }
        }
    }
}

std::vector<int> PlanningTask::get_possible_actions_idx(std::vector<int> &current_state, bool check) {
    std::vector<int> action_idx;
    for (int i = 0; i < this->n_actions; i++) {
        Action action = this->actions[i];
        if (!check && action.is_used)
            continue; // skip actions already used
        int j;
        for (j = 0; j < action.n_preconds; j++)
            if (current_state[action.preconds[j].var_idx] != action.preconds[j].var_val)
                break;
        if (j == action.n_preconds)
            action_idx.push_back(i);
    }
    return action_idx;
}

void PlanningTask::apply_action(int idx, std::vector<int> &current_state) {
    int applied_effects = 0;
    Action action = this->actions[idx];
    for (int i = 0; i < action.n_effects; i++) {
        Effect effect = action.effects[i];
        int j;
        for (j = 0; j < effect.n_effect_conds; j++) {
            std::vector<Fact> effect_conds = effect.effect_conds;
            if (current_state[effect_conds[j].var_idx] != effect_conds[j].var_val &&
                effect_conds[j].var_val != -1)
                break;
        }
        if (j < effect.n_effect_conds) // the effect cannot be applied
            continue;
        int var = effect.var_affected;
        if ((current_state[var] == effect.from_value ||
            effect.from_value == -1) && check_mutex_groups(var, effect.to_value, current_state)) {
            current_state[var] = effect.to_value;
            applied_effects++;
            if (applied_effects == 1) {
                IndexAction indexAction;
                indexAction.idx = idx;
                indexAction.action = action;
                this->solution.push_back(indexAction);
                if (this->metric == 1)
                    this->solution_cost += action.cost;
                else
                    this->solution_cost += 1;
            }
        }
    }
    if (applied_effects == action.n_effects)
        action.is_used = true; // an action is used only if all the effects are applied once
}

void PlanningTask::print_solution() {
    for (int i = 0; i < this->solution.size(); i++) {
        std::cout << this->solution[i].idx << ": " + this->solution[i].action.name << std::endl;
    }
    std::cout << "Cost: " << this->solution_cost << std::endl;
}

void PlanningTask::brute_force(int seed) {
    srand(seed);
    std::vector<int> current_state = this->initial_state;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);
        std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state, false);
        if (possible_actions_idx.empty())
            break;
        int action_to_apply_idx = possible_actions_idx[PlanningTaskUtils::get_random_number(0, possible_actions_idx.size())];
        apply_action(action_to_apply_idx, current_state);
    }

    if (goal_reached(current_state))
        std::cout << "Solution found" << std::endl;
    else
        std::cout << "Goal state not reached" << std::endl;
}

std::vector<int> PlanningTask::get_min_cost_actions_idx(std::vector<int> &actions_idx) {
    std::vector<int> res;
    int min_cost = this->actions[actions_idx[0]].cost;

    // find minimum cost
    for (int i = 1; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].cost < min_cost)
            min_cost = this->actions[idx].cost;
    }

    // get all minimum cost action indexes
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].cost == min_cost)
            res.push_back(idx);
    }

    return res;
}

void PlanningTask::greedy(int seed) {
    if (this->metric == 0) { // it is equivalent to brute_force
        std::cout << "Executing brute force" << std::endl;
        brute_force(seed);
    } else {
        srand(seed);
        std::vector<int> current_state = this->initial_state;

        while (!goal_reached(current_state)) {
            apply_axioms(current_state);
            std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state, false);
            if (possible_actions_idx.empty())
                break;
            std::vector<int> min_cost_actions_idx = get_min_cost_actions_idx(possible_actions_idx);
            int action_to_apply_idx = min_cost_actions_idx[PlanningTaskUtils::get_random_number(0, min_cost_actions_idx.size())];
            apply_action(action_to_apply_idx, current_state);
        }

        if (goal_reached(current_state))
            std::cout << "Solution found" << std::endl;
        else
            std::cout << "Goal state not reached" << std::endl;
    }
}

std::vector<int> PlanningTask::get_min_h_cost_actions_idx(std::vector<int> &actions_idx) {
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
        if (this->actions[idx].h_cost == min_cost)
            res.push_back(idx);
    }

    return res;
}

void PlanningTask::print_action_h_costs(std::vector<int> &actions_idx) {
    std::cout << "################################## Action h costs:" << std::endl;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        std::cout << this->actions[idx].name << " -> " << this->actions[idx].h_cost << std::endl;
    }
}

std::vector<int> PlanningTask::get_actions_idx_having_outcome(Fact &fact) {
    std::vector<int> actions_idx;

    for (int j = 0; j < this->n_actions; j++) {
        Action action = this->actions[j];
        for (int k = 0; k < action.n_effects; k++) {
            Effect effect = action.effects[k];
            if (effect.var_affected == fact.var_idx && effect.to_value == fact.var_val) {
                actions_idx.push_back(j);
            }
        }
    }

    return actions_idx;
}

int PlanningTask::h_add(std::vector<int> &current_state, Fact &fact, std::set<int> &visited) {
    if (current_state[fact.var_idx] == fact.var_val || visited.find(fact.var_idx) != visited.end())
        return 0; // base case

    visited.insert(fact.var_idx);

    // get all the actions having "fact" as outcome
    std::vector<int> actions_idx = get_actions_idx_having_outcome(fact);

    if (actions_idx.empty()) // the fact is unreachable
        return std::numeric_limits<int>::max();

    std::vector<int> h_costs;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];

        if (this->metric == 1)
            this->actions[idx].h_cost = this->actions[idx].cost;
        else
            this->actions[idx].h_cost = 1;

        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            this->actions[idx].h_cost += h_add(current_state, this->actions[idx].preconds[j], visited);
        }

        h_costs.push_back(this->actions[idx].h_cost);
    }

    return *std::min_element(h_costs.begin(), h_costs.end());
}

int PlanningTask::h_max(std::vector<int> &current_state, Fact &fact, std::set<int> &visited) {
    if (current_state[fact.var_idx] == fact.var_val || visited.find(fact.var_idx) != visited.end())
        return 0; // base case

    visited.insert(fact.var_idx);

    // get all the actions having "fact" as outcome
    std::vector<int> actions_idx = get_actions_idx_having_outcome(fact);

    if (actions_idx.empty()) // the fact is unreachable
        return std::numeric_limits<int>::max();

    std::vector<int> h_costs;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];

        if (this->metric == 1)
            this->actions[idx].h_cost = this->actions[idx].cost;
        else
            this->actions[idx].h_cost = 1;

        int max_cost = 0;
        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            max_cost = std::max(max_cost, h_max(current_state, this->actions[idx].preconds[j], visited));
        }

        this->actions[idx].h_cost += max_cost;
        h_costs.push_back(this->actions[idx].h_cost);
    }

    return *std::min_element(h_costs.begin(), h_costs.end());
}

int PlanningTask::compute_heuristic(std::vector<int> &current_state, int heuristic) {
    int total = 0;
    for (int i = 0; i < this->n_goals; i++) {
        std::set<int> visited;
        if (heuristic == 2)
            total += h_add(current_state, this->goal_state[i], visited);
        else if (heuristic == 3)
            total = std::max(total, h_max(current_state, this->goal_state[i], visited));
            // total += h_max(current_state, this->goal_state[i], visited); TODO
    }
    return total;
}

void PlanningTask::remove_satisfied_actions(std::vector<int> &current_state, std::vector<int> &possible_actions_idx) {
    for (int i = possible_actions_idx.size() - 1; i >= 0; i--) {
        int idx = possible_actions_idx[i];
        std::vector<Effect> effects = this->actions[idx].effects;
        int count = 0;
        for (int j = 0; j < effects.size(); j++) {
            if (current_state[effects[j].var_affected] == effects[j].to_value)
                count++;
        }
        if (count == effects.size()){
            possible_actions_idx.erase(possible_actions_idx.begin() + i);
            this->actions[idx].is_used = true; // this action shouldn't be returned anymore
        }
    }
}

void PlanningTask::solve(int seed, int heuristic) {
    srand(seed);
    std::vector<int> current_state = this->initial_state;
    int estimated_cost = std::numeric_limits<int>::max();

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);

        // calculate heuristic costs
        int total = compute_heuristic(current_state, heuristic);
        if (total < estimated_cost) {
            estimated_cost = total;
            std::cout << "New estimated cost to reach the goal state: " << estimated_cost << std::endl;
        }

        // get possible actions
        std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state, false);

        // remove actions having outcome already satisfied
        remove_satisfied_actions(current_state, possible_actions_idx);

        // get min h cost actions
        std::vector<int> min_h_cost_actions_idx = get_min_h_cost_actions_idx(possible_actions_idx);

        // apply action
        int action_to_apply_idx = min_h_cost_actions_idx[PlanningTaskUtils::get_random_number(0, min_h_cost_actions_idx.size())];
        apply_action(action_to_apply_idx, current_state);
    }

    std::cout << "Solution found!" << std::endl;
}

void PlanningTask::remove_previous_state_actions(std::vector<int> &actions_idx, std::vector<std::vector<int>> &previous_actions_idx) {
    for (int i = actions_idx.size() - 1; i >= 0; i--) {
        for (int j = 0; j < previous_actions_idx.size(); j++) {
            for (int k = 0; k < previous_actions_idx[j].size(); k++) {
                if (actions_idx[i] == previous_actions_idx[j][k])
                    actions_idx.erase(actions_idx.begin() + i);
            }
        }
    }
}

void PlanningTask::compute_graph() {
    // structure initialization
    this->graph_states.push_back(this->initial_state);

    int graph_layer = 0;
    while (!goal_reached(this->graph_states.back())) {
        std::vector<int> current_state = this->graph_states.back();

        std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state, false); // here we have also the previous state actions
        if (!actions.empty())
            remove_previous_state_actions(possible_actions_idx, this->graph_actions);

        // assign to each action the graph layer
        for (int i = 0; i < possible_actions_idx.size(); i++) {
            this->actions[possible_actions_idx[i]].graph_layer = graph_layer;
        }
        graph_layer++;
        this->graph_actions.push_back(possible_actions_idx);

        // get all the possible outcomes applying all the actions
        for (int k = 0; k < possible_actions_idx.size(); k++) {
            int idx = possible_actions_idx[k];
            Action action = this->actions[idx];
            for (int i = 0; i < action.n_effects; i++) {
                Effect effect = action.effects[i];
                int j;
                for (j = 0; j < effect.n_effect_conds; j++) {
                    std::vector<Fact> effect_conds = effect.effect_conds;
                    if (current_state[effect_conds[j].var_idx] != effect_conds[j].var_val &&
                        effect_conds[j].var_val != -1)
                        break;
                }
                if (j < effect.n_effect_conds) // the effect cannot be applied
                    continue;
                int var = effect.var_affected;
                if ((current_state[var] == effect.from_value ||
                    effect.from_value == -1) && check_mutex_groups(var, effect.to_value, current_state)) {
                    current_state[var] = effect.to_value;
                }
            }
        }
        this->graph_states.push_back(current_state);
    }
}

bool PlanningTask::check_integrity() {
    std::vector<int> current_state = this->initial_state;
    for (int k = 0; k < this->solution.size(); k++) {
        IndexAction indexAction = this->solution[k];
        std::cout << "Current action: " << indexAction.idx << std::endl;
        std::vector<int> actions_idx = get_possible_actions_idx(current_state, true);
        std::cout << "Possible actions: " << std::endl;
        PlanningTaskUtils::print_planning_task_state(actions_idx);
        int p = 0;
        for (; p < actions_idx.size(); p++) {
            if (actions_idx[p] == indexAction.idx)
                break;
        }
        if (p == actions_idx.size())
            return false; // action not applicable at this point
        for (int i = 0; i < indexAction.action.n_effects; i++) {
            Effect effect = indexAction.action.effects[i];
            int j;
            for (j = 0; j < effect.n_effect_conds; j++) {
                std::vector<Fact> effect_conds = effect.effect_conds;
                if (current_state[effect_conds[j].var_idx] != effect_conds[j].var_val &&
                    effect_conds[j].var_val != -1)
                    break;
            }
            if (j < effect.n_effect_conds) // the effect cannot be applied
                continue;
            int var = effect.var_affected;
            if ((current_state[var] == effect.from_value ||
                effect.from_value == -1) && check_mutex_groups(var, effect.to_value, current_state)) {
                current_state[var] = effect.to_value;
            }
        }
    }
    return true;
}

bool PlanningTask::action_in_graph(int action_idx) {
    for (int layer = 0; layer < this->graph_actions.size(); layer++) {
        std::vector<int> actions = this->graph_actions[layer];
        for (int i = 0; i < actions.size(); i++) {
            if (action_idx == actions[i])
                return true;
        }
    }
    return false;
}

void PlanningTask::adjust_plan() {
    for (int i = 0; i < this->solution.size(); i++) {
        if (action_in_graph(this->solution[i].idx)) // action contained in the graph? nothing to do
            continue;
        printf("%d\n", this->solution[i].idx);
    }
}
