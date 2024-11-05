#ifndef PLANNING_TASK_UTILS_H
#define PLANNING_TASK_UTILS_H

#include "planning_task.h"
#include <vector>

namespace PlanningTaskUtils {
    void print_planning_task(PlanningTask &pt);
    void print_planning_task_vars(PlanningTask &pt);
    void print_planning_task_mutexes(PlanningTask &pt);
    void print_planning_task_state(std::vector<int> &state);
    void print_planning_task_goal(PlanningTask &pt);
    void print_planning_task_actions(PlanningTask &pt);
    void print_planning_task_axioms(PlanningTask &pt);

    void print_var(Variable &var);
    void print_mutex(MutexGroup &mutex);
    void print_fact(Fact &fact);
    void print_action(Action &action);
    void print_effect(Effect &effect);
    void print_axiom(Axiom &axiom);

    int get_random_number(int lower, int upper);
}

#endif
