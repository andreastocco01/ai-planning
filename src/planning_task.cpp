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
    if (actions_idx.empty()) return {};

    std::vector<int> res;
    int min_cost = std::numeric_limits<int>::max();

    for (int idx : actions_idx) {
        int cost = this->actions[idx].h_cost;
        if (cost < min_cost) {
            min_cost = cost;
            res.clear();  // New minimum found, discard previous candidates
            res.push_back(idx);
        } else if (cost == min_cost) {
            res.push_back(idx);
        }
    }

    if (min_cost ==
        std::numeric_limits<int>::max())  // all the actions are unreachable
        res.clear();

    return res;
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

void print_queue(std::queue<QueueFrame> q) {
    while (!q.empty()) {
        QueueFrame frame = q.front();
        std::cout << "([" << frame.fact.var_idx << ", " << frame.fact.var_val
                  << "], " << frame.level << ")"
                  << " ";
        q.pop();
    }
    std::cout << std::endl;
}

void print_stack(std::stack<StackFrame> s) {
    int level = s.size() - 1;
    while (!s.empty()) {
        StackFrame frame = s.top();
        std::cout << level-- << ": "
                  << "(" << frame.action_idx << ", [" << frame.from.var_idx
                  << ", " << frame.from.var_val << "], " << frame.level << ")"
                  << std::endl;
        s.pop();
    }
    std::cout << std::endl;
}

void print_array_queue(std::vector<QueueFrame> arr) {
    while (!arr.empty()) {
        QueueFrame frame = arr.front();
        std::cout << "([" << frame.fact.var_idx << ", " << frame.fact.var_val
                  << "], " << frame.level << ")"
                  << " ";
        arr.erase(arr.begin());
    }
    std::cout << std::endl;
}

void print_array_stack(std::vector<StackFrame> arr) {
    int level = arr.size() - 1;
    while (!arr.empty()) {
        StackFrame frame = arr.back();
        std::cout << level-- << ": "
                  << "(" << frame.action_idx << ", [" << frame.from.var_idx
                  << ", " << frame.from.var_val << "], " << frame.level << ")"
                  << std::endl;
        arr.pop_back();
    }
    std::cout << std::endl;
}

int PlanningTask::h_max_optimized(std::vector<int> &current_state) {
    PriorityQueue<int> pq(this->facts.size());
    int inf = std::numeric_limits<int>::max();
    std::vector<int> fact_costs(this->facts.size(), inf);

    // Initialize current state facts
    for (int i = 0; i < current_state.size(); i++) {
        Fact f{i, current_state[i]};
        int idx = FIND_FACT_INDEX(f);
        fact_costs[idx] = 0;
        pq.push(idx, 0);
    }

    // Main loop
    while (!pq.isEmpty()) {
        int fact_idx = pq.top();
        pq.pop();
        Fact f = this->facts[fact_idx];
        std::vector<int> actions_idx = this->map_precond_actions[f];
        // Add actions with no preconditions
        actions_idx.insert(actions_idx.end(), this->actions_no_preconds.begin(),
                           this->actions_no_preconds.end());
        for (int a_idx : actions_idx) {
            Action &action = this->actions[a_idx];
            if (action.is_used) continue;  // skip actions already used
            int max_pre = 0;
            for (const Fact &pre : action.preconds) {
                int pre_idx = FIND_FACT_INDEX(pre);
                max_pre = std::max(max_pre, fact_costs[pre_idx]);
            }
            if (max_pre == inf) continue;
            int new_cost =
                this->metric == 1 ? (max_pre + action.cost) : (max_pre + 1);
            action.h_cost = new_cost;
            for (const Effect &eff : action.effects) {
                Fact eff_fact{eff.var_affected, eff.to_value};
                int eff_idx = FIND_FACT_INDEX(eff_fact);
                if (new_cost < fact_costs[eff_idx]) {
                    fact_costs[eff_idx] = new_cost;
                    if (pq.has(eff_idx))
                        pq.change(eff_idx, new_cost);
                    else
                        pq.push(eff_idx, new_cost);
                }
            }
        }
    }

    // Compute h_max for goals
    int h_max = 0;
    for (const Fact &goal : this->goal_state) {
        h_max = std::max(h_max, fact_costs[FIND_FACT_INDEX(goal)]);
    }
    return h_max;
}

void PlanningTask::create_callstack() {
    for (int i = 0; i < this->n_goals; i++) {
        Fact goal = this->goal_state[i];

        int inf = std::numeric_limits<int>::max();
        std::vector<int> fact_costs(this->facts.size(), inf);

        // Initialize current state facts
        for (int i = 0; i < this->initial_state.size(); i++) {
            Fact f{i, this->initial_state[i]};
            int idx = FIND_FACT_INDEX(f);
            fact_costs[idx] = 0;
        }

        std::vector<QueueFrame> &queue = this->map_fqueue[goal];
        std::vector<StackFrame> &stack = this->map_astack[goal];
        queue.push_back({goal, -1});
        std::vector<bool> visited_actions(this->n_actions, false);
        std::vector<bool> visited_facts(this->facts.size(), false);

        // create the sequence of actions to compute costs
        for (int idx = 0; idx < queue.size(); idx++) {
            Fact current = queue[idx].fact;  // get the oldest fact in the queue
            int level = queue[idx].level;

            if (fact_costs[FIND_FACT_INDEX(current)] == 0) continue;

            // get the actions having the popped fact as effect
            std::vector<int> actions = this->map_effect_actions[current];
            for (int i = 0; i < actions.size(); i++) {
                if (this->actions[actions[i]].is_used ||
                    visited_actions[actions[i]])
                    continue;

                visited_actions[actions[i]] = true;
                // push the pair (action, fact) onto the stack
                stack.push_back({actions[i], current, level});
                Action action = this->actions[actions[i]];
                for (int j = 0; j < action.n_preconds; j++) {
                    int fact_idx = FIND_FACT_INDEX(action.preconds[j]);
                    if (visited_facts[fact_idx]) continue;

                    visited_facts[fact_idx] = true;
                    // push the precondition to the end of the fact queue
                    queue.push_back(
                        {action.preconds[j], (int)stack.size() - 1});
                }
            }
        }

        this->visited_actions[goal] = visited_actions;
        this->visited_facts[goal] = visited_facts;
    }
}

int PlanningTask::iterative_h_max(std::vector<int> &current_state, Fact fact) {
    int inf = std::numeric_limits<int>::max();
    std::vector<int> fact_costs(this->facts.size(), inf);

    // Initialize current state facts
    for (int i = 0; i < current_state.size(); i++) {
        Fact f{i, current_state[i]};
        int idx = FIND_FACT_INDEX(f);
        fact_costs[idx] = 0;
    }

    std::vector<StackFrame> &astack =
        this->map_astack[fact];  // actually compute the costs
    for (int i = astack.size() - 1; i >= 0; i--) {
        StackFrame entry = astack[i];  // get the last inserted action

        Action &action = this->actions[entry.action_idx];

        int max_pre = 0;
        for (int j = 0; j < action.n_preconds; j++) {
            max_pre = std::max(max_pre,
                               fact_costs[FIND_FACT_INDEX(action.preconds[j])]);
        }

        if (max_pre == inf)
            action.h_cost = inf;
        else
            action.h_cost =
                this->metric == 1 ? (action.cost + max_pre) : (1 + max_pre);

        int fact_idx = FIND_FACT_INDEX(entry.from);
        fact_costs[fact_idx] = std::min(fact_costs[fact_idx], action.h_cost);
    }

    return fact_costs[FIND_FACT_INDEX(fact)];
}

void PlanningTask::update_goal_callstack(int applied_action_idx,
                                         std::vector<int> &current_state,
                                         Fact goal) {
    Action action = this->actions[applied_action_idx];

    std::vector<StackFrame> &stack = this->map_astack[goal];
    std::vector<QueueFrame> &queue = this->map_fqueue[goal];
    std::vector<bool> &visited_actions = this->visited_actions[goal];
    std::vector<bool> &visited_facts = this->visited_facts[goal];

    int idx = 0;

    if (!stack.empty()) {  // the stack is not empty
        int stack_level = stack.size();
        Fact fact_to_remove;
        // find the last occurrence among the added facts on the stack
        for (int i = 0; i < action.n_effects; ++i) {
            Fact f{action.effects[i].var_affected, action.effects[i].to_value};

            for (size_t j = 0; j < stack.size(); ++j) {
                if (stack[j].from == f && j < (size_t)stack_level) {
                    stack_level = j;
                    fact_to_remove = f;
                    break;  // no need to look further for this fact
                }
            }
        }

        // remove all the elements above in the stack and in the queue
        if (stack_level < stack.size()) {
            // unmark visited actions above stack_level
            for (size_t i = stack_level; i < stack.size(); ++i) {
                visited_actions[stack[i].action_idx] = false;
            }

            // unmark visited facts with level >= stack_level
            auto queue_cutoff = std::find_if(
                queue.begin(), queue.end(),
                [&](const QueueFrame &qf) { return qf.level >= stack_level; });

            for (auto it = queue_cutoff; it != queue.end(); ++it) {
                visited_facts[FIND_FACT_INDEX(it->fact)] = false;
            }

            // erase elements to be recomputed
            stack.erase(stack.begin() + stack_level, stack.end());
            queue.erase(queue_cutoff, queue.end());

            // find first fact not computed in the queue
            idx = std::distance(queue.begin(),
                                std::find_if(queue.begin(), queue.end(),
                                             [&](const QueueFrame &frame) {
                                                 return frame.fact ==
                                                        stack.back().from;
                                             }));
        }
    }

    // add the new layers
    int inf = std::numeric_limits<int>::max();
    std::vector<int> fact_costs(this->facts.size(), inf);

    // Initialize current state facts
    for (int i = 0; i < current_state.size(); i++) {
        Fact f{i, current_state[i]};
        int idx = FIND_FACT_INDEX(f);
        fact_costs[idx] = 0;
    }

    // recompute layers
    for (; idx < queue.size(); idx++) {
        Fact current = queue[idx].fact;  // get the oldest fact in the queue
        int level = queue[idx].level;

        if (fact_costs[FIND_FACT_INDEX(current)] == 0) continue;

        // get the actions having the popped fact as effect
        std::vector<int> actions = this->map_effect_actions[current];
        for (int i = 0; i < actions.size(); i++) {
            if (this->actions[actions[i]].is_used ||
                visited_actions[actions[i]])
                continue;

            visited_actions[actions[i]] = true;
            // push the pair (action, fact) onto the stack
            stack.push_back({actions[i], current, level});
            Action action = this->actions[actions[i]];
            for (int j = 0; j < action.n_preconds; j++) {
                int fact_idx = FIND_FACT_INDEX(action.preconds[j]);
                if (visited_facts[fact_idx]) continue;

                visited_facts[fact_idx] = true;
                // push the precondition to the end of the fact queue
                queue.push_back({action.preconds[j], (int)stack.size() - 1});
            }
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

int PlanningTask::h_add(std::vector<int> &current_state, Fact &fact,
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

    // get all the actions having "fact" as outcome
    std::vector<int> actions_idx = this->map_effect_actions[fact];

    if (actions_idx.empty())  // the fact is unreachable
        return std::numeric_limits<int>::max();

    int min_h_cost = std::numeric_limits<int>::max();

    for (int idx : actions_idx) {
        if (this->actions[idx].is_used) continue;
        if (this->metric == 1)
            this->actions[idx].h_cost = this->actions[idx].cost;
        else
            this->actions[idx].h_cost = 1;

        for (int j = 0; j < this->actions[idx].n_preconds; j++) {
            this->actions[idx].h_cost += h_add(
                current_state, this->actions[idx].preconds[j], visited, cache);
        }

        if (this->actions[idx].h_cost < 0)  // overflow
            this->actions[idx].h_cost = std::numeric_limits<int>::max();

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

    if (heuristic == 2) {
        total = h_max_optimized(current_state);
    } else if (heuristic == 4) {
        for (int i = 0; i < this->n_goals; i++) {
            std::unordered_set<int> visited;
            total += h_add(current_state, this->goal_state[i], visited, cache);
        }
        if (total < 0)  // overflow
            total = std::numeric_limits<int>::max();
    } else if (heuristic == 5) {
        for (int i = 0; i < this->n_goals; i++) {
            std::unordered_set<int> visited;
            total = std::max(total, h_max(current_state, this->goal_state[i],
                                          visited, cache));
        }
    } else if (heuristic == 6) {
        for (int i = 0; i < this->n_goals; i++) {
            total = std::max(
                total, iterative_h_max(current_state, this->goal_state[i]));
        }
    }

    return total;
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

    if (heuristic == 6) create_callstack();

    bool no_solution = false;

    while (!goal_reached(current_state)) {
        apply_axioms(current_state);

        // calculate heuristic costs
        if (heuristic == 2 || heuristic == 4 || heuristic == 5 ||
            heuristic == 6) {
            reset_actions_metadata();
            int total = compute_heuristic(current_state, heuristic);
            if (total < estimated_cost) {
                estimated_cost = total;
                std::cout << "New estimated cost to reach the goal state: "
                          << estimated_cost << std::endl;
            }
        }

        // get possible actions
        std::vector<int> possible_actions_idx =
            get_possible_actions_idx(current_state, true);

        /*std::cout << "POSSIBLE ACTIONS: ";
        PlanningTaskUtils::print_planning_task_state(possible_actions_idx);
        print_action_h_costs(possible_actions_idx);*/

        // remove actions having outcome already satisfied
        remove_satisfied_actions(current_state, possible_actions_idx);

        if (possible_actions_idx.empty()) {
            no_solution = true;
            break;
        }

        // apply each possible action
        // re-compute hmax
        // add to h_cost the result of hmax
        if (heuristic == 5) {
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
                        if (new_state[effect_cond.var_idx] !=
                                effect_cond.var_val &&
                            effect_cond.var_val != -1)
                            break;
                    }
                    if (j <
                        effect.n_effect_conds)  // the effect cannot be applied
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
                costs[k] = total + costs[k] < 0
                               ? std::numeric_limits<int>::max()
                               : total + costs[k];
            }

            for (int i = 0; i < possible_actions_idx.size(); i++)
                this->actions[possible_actions_idx[i]].h_cost = costs[i];
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

            if (min_h_cost_actions_idx.empty()) {
                no_solution = true;
                break;
            }
            action_to_apply_idx =
                min_h_cost_actions_idx[PlanningTaskUtils::get_random_number(
                    0, min_h_cost_actions_idx.size())];
        }

        // std::cout << "APPLIED ACTION: " << action_to_apply_idx << std::endl;
        // apply action
        apply_action(action_to_apply_idx, current_state);

        if (heuristic == 6)
            for (int i = 0; i < this->n_goals; i++)
                update_goal_callstack(action_to_apply_idx, current_state,
                                      this->goal_state[i]);
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
