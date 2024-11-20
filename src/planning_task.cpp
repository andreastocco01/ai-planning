#include "../include/planning_task.h"
#include "../include/planning_task_utils.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cassert>
#include <iterator>
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

std::vector<int> PlanningTask::get_possible_actions_idx(std::vector<int> &current_state) {
    std::vector<int> action_idx;
    for (int i = 0; i < this->n_actions; i++) {
        Action action = this->actions[i];
        if (action.is_used)
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

void PlanningTask::apply_action(Action &action, std::vector<int> &current_state) {
    int applied_effects = 0;
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
                this->solution.push_back(action);
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
        std::cout << this->solution[i].name << std::endl;
    }
    std::cout << "Cost: " << this->solution_cost << std::endl;
}

void PlanningTask::brute_force(int seed) {
    srand(seed);
    std::vector<int> current_state = this->initial_state;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);
        std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state);
        if (possible_actions_idx.empty())
            break;
        int action_to_apply_idx = possible_actions_idx[PlanningTaskUtils::get_random_number(0, possible_actions_idx.size())];
        apply_action(this->actions[action_to_apply_idx], current_state);
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
            std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state);
            if (possible_actions_idx.empty())
                break;
            std::vector<int> min_cost_actions_idx = get_min_cost_actions_idx(possible_actions_idx);
            int action_to_apply_idx = min_cost_actions_idx[PlanningTaskUtils::get_random_number(0, min_cost_actions_idx.size())];
            apply_action(this->actions[action_to_apply_idx], current_state);
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

    // find minimum cost
    for (int i = 1; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].h_cost < min_cost)
            min_cost = this->actions[idx].h_cost;
    }

    // get all minimum cost action indexes
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        if (this->actions[idx].h_cost == min_cost)
            res.push_back(idx);
    }

    return res;
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
    std::cout << "Actions having outcome: " << fact.var_idx << " " << fact.var_val << std::endl;
    PlanningTaskUtils::print_planning_task_state(actions_idx);

    std::vector<int> h_costs;
    for (int i = 0; i < actions_idx.size(); i++) {
        int idx = actions_idx[i];
        h_costs.push_back(this->actions[idx].cost);
        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            h_costs[h_costs.size() - 1] += h_add(current_state, this->actions[idx].preconds[j], visited);
        }
        this->actions[idx].h_cost = h_costs.back();
    }

    auto minIt = std::min_element(h_costs.begin(), h_costs.end());
    int min = *minIt;

    return min;
}

void PlanningTask::compute_h_add(std::vector<int> &current_state) {
    for (int i = 0; i < this->n_goals; i++) {
        std::set<int> visited;
        h_add(current_state, this->goal_state[i], visited);
    }
}

void PlanningTask::solve(int seed) {
    srand(seed);
    std::vector<int> current_state = this->initial_state;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);

        // calculate heuristic costs
        compute_h_add(current_state);

        // get possible actions
        std::vector<int> possible_actions_idx = get_possible_actions_idx(current_state);

        // get min h cost actions
        std::vector<int> min_h_cost_actions_idx = get_min_h_cost_actions_idx(possible_actions_idx);

        // apply action
        int action_to_apply_idx = min_h_cost_actions_idx[PlanningTaskUtils::get_random_number(0, min_h_cost_actions_idx.size())];
        apply_action(this->actions[action_to_apply_idx], current_state);
    }

    std::cout << "Solution found!" << std::endl;
}
