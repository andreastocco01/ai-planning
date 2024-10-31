#include "../include/planning_task_utils.h"
#include <cstdlib>
#include <iostream>

void PlanningTaskUtils::print_planning_task_vars(PlanningTask &pt) {
    for (int i = 0; i < pt.n_vars; i++) {
        for (int j = 0; j < pt.vars[i].range; j ++) {
            std::cout << pt.vars[i].sym_names[j] << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task_facts(PlanningTask &pt) {
    for (int i = 0; i < pt.n_mutex; i++) {
        for (int j = 0; j < pt.mutexes[i].n_facts; j++) {
            std::cout << pt.mutexes[i].facts[j].var_idx << " " << pt.mutexes[i].facts[j].var_val << std::endl;
        }
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task_initial_state(PlanningTask &pt) {
    for (int i = 0; i < pt.n_vars; i++) {
        std::cout << pt.initial_state[i] << std::endl;
    }
    std::cout << std::endl;
}

void PlanningTaskUtils::print_planning_task_goal(PlanningTask &pt) {
    for (int i = 0; i < pt.n_goals; i++) {
        std::cout << pt.goal_state[i].var_idx << " " << pt.goal_state[i].var_val << std::endl;
    }
    std::cout << std::endl;
}

void PlanningTaskUtils::print_planning_task_actions(PlanningTask &pt) {
    for (int i = 0; i < pt.n_actions; i++) {
        std::cout << pt.actions[i].name << std::endl;
        std::cout << pt.actions[i].n_preconds << std::endl;

        for (int j = 0; j < pt.actions[i].n_preconds; j++) {
            std::cout << pt.actions[i].preconds[j].var_idx << " " << pt.actions[i].preconds[j].var_val << std::endl;
        }

        std::cout << pt.actions[i].n_effects << std::endl;

        for (int j = 0; j < pt.actions[i].n_effects; j++) {
            std::cout << pt.actions[i].effects[j].n_effect_conds << " ";
            for (int k = 0; k < pt.actions[i].effects[j].n_effect_conds; k++) {
                std::cout << pt.actions[i].effects[j].effect_conds[k].var_idx << " "
                << pt.actions[i].effects[j].effect_conds[k].var_val << " ";
            }
            std::cout << pt.actions[i].effects[j].var_affected << " "
            << pt.actions[i].effects[j].from_value << " "
            << pt.actions[i].effects[j].to_value << std::endl;
        }
        std::cout << pt.actions[i].cost << std::endl;
        std::cout << std::endl;
    }
}

void PlanningTaskUtils::print_planning_task_axioms(PlanningTask &pt) {
    for (int i = 0; i < pt.n_axioms; i++) {
        std::cout << pt.axioms[i].n_conds << std::endl;
        for (int j = 0; j < pt.axioms[i].n_conds; j++) {
            std::cout << pt.axioms[i].conds[j].var_idx << " "
            << pt.axioms[i].conds[j].var_val << std::endl;
        }
        std::cout << pt.axioms[i].affected_var << " "
        << pt.axioms[i].from_value << " "
        << pt.axioms[i].to_value << std::endl;
        std::cout << std::endl;
    }

}

void PlanningTaskUtils::print_planning_task(PlanningTask &pt) {
    std::cout << "Metric: " << pt.metric << std::endl;
    std::cout << std::endl;
    std::cout << "Variables:" << std::endl;
    print_planning_task_vars(pt);
    std::cout << "Facts:" << std::endl;
    print_planning_task_facts(pt);
    std::cout << "Initial state:" << std::endl;
    print_planning_task_initial_state(pt);
    std::cout << "Goal state:" << std::endl;
    print_planning_task_goal(pt);
    std::cout << "Actions:" << std::endl;
    print_planning_task_actions(pt);
    std::cout << "Axioms:" << std::endl;
    print_planning_task_axioms(pt);
}

int PlanningTaskUtils::get_random_number(int lower, int upper) {
    return lower + rand() % (upper - lower);
}
