#ifndef PLANNING_TASK_UTILS_H
#define PLANNING_TASK_UTILS_H

#include "planning_task.h"

namespace PlanningTaskUtils {
    void print_planning_task(PlanningTask &pt);
    void print_planning_task_vars(PlanningTask &pt);
    void print_planning_task_facts(PlanningTask &pt);
    void print_planning_task_initial_state(PlanningTask &pt);
    void print_planning_task_goal(PlanningTask &pt);
    void print_planning_task_actions(PlanningTask &pt);
    void print_planning_task_axioms(PlanningTask &pt);

    int get_random_number(int lower, int upper);
}

#endif
