#include "../include/planning_task.h"
#include "../include/planning_task_utils.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <ostream>
#include <string>
#include <sstream>

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

bool PlanningTask::check_axiom_cond(Axiom axiom, std::vector<int> &current_state) {
    for (int i = 0; i < axiom.n_conds; i++) {
        if (current_state[axiom.conds[i].var_idx] != axiom.conds[i].var_val)
            return false;
    }
    return true;
}

/*
    check if apply the axiom would break a mutex
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

void PlanningTask::apply_axioms(std::vector<int> &current_state) {
    int max_axiom_layer = get_max_axiom_layer();
    for (int axiom_layer = 0; axiom_layer <= max_axiom_layer; axiom_layer++) {
        for (int i = 0; i < this->n_axioms; i++){
            Axiom axiom = this->axioms[i];
            if (this->vars[axiom.affected_var].axiom_layer == axiom_layer && check_axiom_cond(axiom, current_state)) {
                if ((current_state[axiom.affected_var] == axiom.from_value || current_state[axiom.affected_var] == -1) &&
                    check_mutex_groups(axiom.affected_var, axiom.to_value, current_state)) {
                    current_state[axiom.affected_var] = axiom.to_value;
                }
            }
        }
    }
}

std::vector<Action> PlanningTask::get_possible_actions(std::vector<int> &current_state) {
    std::vector<Action> actions;
    for (int i = 0; i < this->n_actions; i++) {
        Action action = this->actions[i];
        int j;
        for (j = 0; j < action.n_preconds; j++)
            if (current_state[action.preconds[j].var_idx] != action.preconds[j].var_val)
                break;
        if (j == action.n_preconds && !action.is_used) {
            this->actions[i].is_used = true;
            actions.push_back(action);
        }
    }
    return actions;
}

void PlanningTask::apply_action(Action action, std::vector<int> &current_state) {
    for (int i = 0; i < action.n_effects; i++) {
        int j;
        for (j = 0; j < action.effects[i].n_effect_conds; j++) {
            std::vector<Fact> effect_conds = action.effects[i].effect_conds;
            if (current_state[effect_conds[j].var_idx] != effect_conds[j].var_val &&
                current_state[effect_conds[j].var_idx] != -1)
                break;
        }
        if (j < action.effects[i].n_effect_conds)
            continue;
        int var = action.effects[i].var_affected;
        if (current_state[var] == action.effects[i].from_value ||
            action.effects[i].from_value == -1) {
            current_state[var] = action.effects[i].to_value;
            this->solution.push_back(action);
            if (this->metric == 1)
                this->solution_cost += action.cost;
            else
                this->solution_cost += 1;
        }
    }
}

void PlanningTask::brute_force() {
    srand(0);
    std::vector<int> current_state = this->initial_state;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);
        std::vector<Action> possible_actions = get_possible_actions(current_state);
        apply_action(possible_actions[PlanningTaskUtils::get_random_number(0, possible_actions.size())], current_state);
    }

    std::cout << "Solution cost: " << this->solution_cost << std::endl;
}
