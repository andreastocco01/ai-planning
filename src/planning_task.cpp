#include "../include/planning_task.h"
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

void PlanningTask::greedy() {
    std::vector<int> current_state = this->initial_state;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);
    }
}
