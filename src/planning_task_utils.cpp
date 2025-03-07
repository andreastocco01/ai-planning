#include "../include/planning_task_utils.h"

#include <cstdlib>
#include <iostream>
#include <vector>

void PlanningTaskUtils::print_var(Variable &var) {
    std::cout << var.name << std::endl;
    std::cout << var.axiom_layer << std::endl;
    std::cout << var.range << std::endl;
    for (int i = 0; i < var.range; i++) {
        std::cout << var.sym_names[i] << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task_vars(PlanningTask &pt) {
    std::cout << "# Variables: " << pt.n_vars << std::endl;
    for (int i = 0; i < pt.n_vars; i++) {
        print_var(pt.vars[i]);
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_fact(Fact &fact) {
    std::cout << fact.var_idx << " " << fact.var_val << std::endl;
}

void PlanningTaskUtils::print_mutex(MutexGroup &mutex) {
    std::cout << mutex.n_facts << std::endl;
    for (int i = 0; i < mutex.n_facts; i++) {
        print_fact(mutex.facts[i]);
    }
}

void PlanningTaskUtils::print_planning_task_mutexes(PlanningTask &pt) {
    std::cout << "# Mutexes: " << pt.n_mutex << std::endl;
    for (int i = 0; i < pt.n_mutex; i++) {
        print_mutex(pt.mutexes[i]);
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task_state(std::vector<int> &state) {
    for (int i = 0; i < state.size(); i++) {
        std::cout << state[i] << " ";
    }
    std::cout << std::endl;
}

void PlanningTaskUtils::print_planning_task_goal(PlanningTask &pt) {
    std::cout << pt.n_goals << std::endl;
    for (int i = 0; i < pt.n_goals; i++) {
        print_fact(pt.goal_state[i]);
    }
    std::cout << std::endl;
}

void PlanningTaskUtils::print_effect(Effect &effect) {
    std::cout << effect.n_effect_conds << " ";
    for (int i = 0; i < effect.n_effect_conds; i++) {
        std::cout << effect.effect_conds[i].var_idx << " "
                  << effect.effect_conds[i].var_val << " ";
    }
    std::cout << effect.var_affected << " " << effect.from_value << " "
              << effect.to_value << std::endl;
}

void PlanningTaskUtils::print_action(Action &action) {
    std::cout << action.name << std::endl;
    std::cout << action.n_preconds << std::endl;

    for (int i = 0; i < action.n_preconds; i++) {
        print_fact(action.preconds[i]);
    }

    std::cout << action.n_effects << std::endl;

    for (int i = 0; i < action.n_effects; i++) {
        print_effect(action.effects[i]);
    }
    std::cout << action.cost << std::endl;
}

void PlanningTaskUtils::print_planning_task_actions(PlanningTask &pt) {
    std::cout << "# Actions: " << pt.n_actions << std::endl;
    for (int i = 0; i < pt.n_actions; i++) {
        print_action(pt.actions[i]);
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_axiom(Axiom &axiom) {
    std::cout << axiom.n_conds << std::endl;
    for (int i = 0; i < axiom.n_conds; i++) {
        print_fact(axiom.conds[i]);
    }
    std::cout << axiom.affected_var << " " << axiom.from_value << " "
              << axiom.to_value << std::endl;
}

void PlanningTaskUtils::print_planning_task_axioms(PlanningTask &pt) {
    std::cout << "# Axioms: " << pt.n_axioms << std::endl;
    for (int i = 0; i < pt.n_axioms; i++) {
        print_axiom(pt.axioms[i]);
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task(PlanningTask &pt) {
    std::cout << "Metric: " << pt.metric << std::endl;
    std::cout << std::endl;
    std::cout << "Variables:" << std::endl;
    print_planning_task_vars(pt);
    std::cout << "Mutexes:" << std::endl;
    print_planning_task_mutexes(pt);
    std::cout << "Initial state:" << std::endl;
    print_planning_task_state(pt.initial_state);
    std::cout << "Goal state:" << std::endl;
    print_planning_task_goal(pt);
    std::cout << "Actions:" << std::endl;
    print_planning_task_actions(pt);
    std::cout << "Axioms:" << std::endl;
    print_planning_task_axioms(pt);
}

void PlanningTaskUtils::print_structure(PlanningTask &pt) {
    std::cout << "Metric: " << pt.metric << std::endl;
    std::cout << "Variables: " << pt.n_vars << std::endl;
    std::cout << "Mutexes: " << pt.n_mutex << std::endl;
    std::cout << "Actions: " << pt.n_actions << std::endl;
    std::cout << "Axioms: " << pt.n_axioms << std::endl;
}

int PlanningTaskUtils::get_random_number(int lower, int upper) {
    return lower + rand() % (upper - lower);
}
