#include <csignal>
#include <iostream>
#include <string>

#include "include/planning_task.h"
#include "include/planning_task_parser.h"
#include "include/planning_task_utils.h"

void print_usage(std::string executable) {
    std::cerr << "Usage: " << executable
              << " --from-file <file_name> --alg <alg_code> --seed <int> "
                 "[--timelimit <int>] --debug <bool> [--start <float>] [--end "
                 "<float>]"
              << std::endl;
    std::cerr << std::endl
              << "Supported alg_code are:" << std::endl
              << "0: random" << std::endl
              << "1: greedy" << std::endl
              << "2: greedy + pruning" << std::endl
              << "3: h_max + lookahead" << std::endl
              << "4: backward cost propagation (min)" << std::endl
              << "5: backward cost propagation (max)" << std::endl
              << "6: backward cost propagation (sum)" << std::endl
              << "7: re-apply alg 4" << std::endl;
}

void compute_next_state(PlanningTask& pt, int action_idx,
                        std::vector<int>& current_state) {
    for (int i = 0; i < pt.actions[action_idx].n_effects; i++) {
        Effect effect = pt.actions[action_idx].effects[i];
        int j;
        for (j = 0; j < effect.n_effect_conds; j++) {
            Fact effect_cond = effect.effect_conds[j];
            if (current_state[effect_cond.var_idx] != effect_cond.var_val &&
                effect_cond.var_val != -1)
                break;
        }
        if (j < effect.n_effect_conds) {  // the effect cannot be applied
            pt.pending_effects.push_back(effect);
            continue;
        }
        int var = effect.var_affected;
        if (current_state[var] == effect.from_value ||
            effect.from_value == -1) {
            current_state[var] = effect.to_value;
        } else {
            pt.pending_effects.push_back(effect);
        }
    }
}

void merge_solutions(int start, int end, PlanningTask& original,
                     PlanningTask& sub) {
    for (int i = start - 1; i >= 0; i--) {
        sub.solution.insert(sub.solution.begin(), original.solution[i]);
        sub.solution_cost +=
            (sub.metric == 1) ? original.actions[original.solution[i].idx].cost
                              : 1;
        sub.actions[original.solution[i].idx].is_used =
            true;  // mark them as used
    }
    for (int i = end; i < original.solution.size(); i++) {
        sub.solution.push_back(original.solution[i]);
        sub.solution_cost +=
            (sub.metric == 1) ? original.actions[original.solution[i].idx].cost
                              : 1;
        sub.actions[original.solution[i].idx].is_used =
            true;  // mark them as used
    }
}

PlanningTask create_subproblem(PlanningTask& orig, int start, int end) {
    PlanningTask sub(orig);
    std::vector<int> current_state = orig.initial_state;

    // new initial_state = state after applying action at start
    int i = 0;
    for (; i < end; i++) {
        if (i == start) sub.initial_state = current_state;
        int idx = orig.solution[i].idx;
        compute_next_state(sub, idx, current_state);
    }

    // new goal_state = state up to end
    for (int k = 0; k < current_state.size(); k++) {
        if (current_state[k] != sub.initial_state[k]) {
            sub.goal_state.push_back({k, current_state[k]});
        }
    }
    sub.n_goals = sub.goal_state.size();
    return sub;
}

PlanningTask pt, sub;
bool solving_sub = false;

void signal_handler(int signum) {
    std::cout << "Timelimit reached" << std::endl;
    if (solving_sub) {  // a solution for the original problem was found
        std::cout << std::endl
                  << "############### Solution ###############" << std::endl;
        pt.print_solution();
    }
    exit(signum);
}

int main(int argc, char** argv) {
    signal(SIGTERM, signal_handler);
    if (argc < 9) {
        print_usage(argv[0]);
        return 1;
    }

    bool from_file_flag = false;
    bool alg_flag = false;
    bool seed_flag = false;
    bool time_limit_flag = false;
    bool debug_flag = false;
    std::string file_name;
    int alg;
    int seed;
    int time_limit;
    int debug;
    float p_start = -1;
    float p_end = 2;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--from-file") {
            from_file_flag = true;
            file_name = argv[++i];
        }
        if (arg == "--alg") {
            alg_flag = true;
            alg = std::stoi(argv[++i]);
        }
        if (arg == "--seed") {
            seed_flag = true;
            seed = std::stoi(argv[++i]);
        }
        if (arg == "--timelimit") {
            time_limit_flag = true;
            time_limit = std::stoi(argv[++i]);
        }
        if (arg == "--debug") {
            debug_flag = true;
            debug = std::stoi(argv[++i]);
        }
        if (arg == "--start") {
            p_start = std::stof(argv[++i]);
        }
        if (arg == "--end") {
            p_end = std::stof(argv[++i]);
        }
    }

    if (!(from_file_flag && alg_flag && seed_flag && debug_flag) || alg < 0 ||
        alg > 8) {
        print_usage(argv[0]);
        return 1;
    }

    if (!time_limit_flag) time_limit = -1;
    if ((alg == 7 || alg == 8) &&
        (p_start < 0 || p_end > 1 || p_start >= p_end)) {
        print_usage(argv[0]);
        std::cerr << std::endl
                  << "For alg 7 and 8: 0 <= start < end <= 1" << std::endl;
        return 1;
    }

    PlanningTaskParser parser;
    pt = parser.parse_from_file(file_name);
    std::cout << "File " << file_name << " parsed!" << std::endl << std::endl;
    std::cout << "############ File structure #############" << std::endl;
    PlanningTaskUtils::print_structure(pt);

    std::cout << std::endl << "Running algorithm: ";

    switch (alg) {
        case 0:
            std::cout << "random" << std::endl;
            break;
        case 1:
            std::cout << "greedy" << std::endl;
            break;
        case 2:
            std::cout << "greedy + pruning" << std::endl;
            break;
        case 3:
            std::cout << "hmax + lookahead" << std::endl;
            break;
        case 4:
            std::cout << "backward cost propagation (min)" << std::endl;
            break;
        case 5:
            std::cout << "backward cost propagation (max)" << std::endl;
            break;
        case 6:
            std::cout << "backward cost propagation (sum)" << std::endl;
            break;
        case 7:
            std::cout << "reapply backward cost propagation (min)" << std::endl;
            break;
        case 8:
            std::cout << "backward cost propagation (min) + dfs" << std::endl;
            break;
    }

    std::cout << "Solving..." << std::endl;
    int res = (alg == 7 || alg == 8) ? pt.solve(seed, 4, debug, time_limit)
                                     : pt.solve(seed, alg, debug, time_limit);
    if (!res) {
        std::cout << "Solution found!" << std::endl;
        if (alg < 7) {
            std::cout << std::endl
                      << "############### Solution ###############"
                      << std::endl;
            pt.print_solution();
        }
    } else {
        std::cout << "Solution does not exist!" << std::endl;
    }

    if ((alg == 7 || alg == 8) && !res) {
        int start = pt.solution.size() * p_start;
        int end = pt.solution.size() * p_end;
        if (start >= end) {
            std::cout << std::endl
                      << "Degenerate subproblem: start >= end" << std::endl;
            std::cout << "Returning original solution" << std::endl;
            std::cout << std::endl
                      << "############### Solution ###############"
                      << std::endl;
            pt.print_solution();
            return 1;
        }

        std::cout << std::endl << "Solving subproblem..." << std::endl;
        solving_sub = true;
        sub = create_subproblem(pt, start, end);

        int res_sub;
        if (alg == 7)
            res_sub = sub.solve(seed, 4, debug, time_limit);
        else
            res_sub = sub.dfs(end - start);

        if (!res_sub) {
            std::cout << std::endl
                      << "############### Sub-Problem Solution ###############"
                      << std::endl;
            sub.print_solution();

            // merge sub-solution with the original one
            merge_solutions(start, end, pt, sub);
            if (sub.solution_cost < pt.solution_cost) {
                std::cout << "Improved solution found!" << std::endl;
                std::cout << std::endl
                          << "############### Solution ###############"
                          << std::endl;
                sub.print_solution();
            } else {
                std::cout << "No improvements. Returning original solution"
                          << std::endl;
                std::cout << std::endl
                          << "############### Solution ###############"
                          << std::endl;
                pt.print_solution();
            }

            if (debug) {
                sub.initial_state = pt.initial_state;
                sub.goal_state = pt.goal_state;
                if (sub.check_integrity())
                    std::cout << "Integrity check passed!" << std::endl;
                else
                    std::cout << "Integrity check NOT passed!" << std::endl;
            }
        }
    }
    return 0;
}
