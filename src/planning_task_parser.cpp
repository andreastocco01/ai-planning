#include "../include/planning_task_parser.h"
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

void PlanningTaskParser::assert_version(std::ifstream &file) {
    std::string line;

    // the translator version must be 3
    getline(file, line);
    assert(line == "begin_version");
    getline(file, line);
    assert(line == "3");
    getline(file, line);
    assert(line == "end_version");
}

int PlanningTaskParser::get_metric(std::ifstream &file) {
    std::string line;

    // the metric must be 0 or 1
    getline(file, line);
    assert(line == "begin_metric");
    getline(file, line);
    int metric = std::stoi(line);
    assert(metric == 0 || metric == 1);
    getline(file, line);
    assert(line == "end_metric");

    return metric;
}

std::vector<Variable> PlanningTaskParser::get_variables(std::ifstream &file) {
    std::vector<Variable> vars;
    std::string line;

    // number of variables
    getline(file, line);
    int n_vars = std::stoi(line);

    for (int i = 0; i < n_vars; i++) {
        getline(file, line);
        assert(line == "begin_variable");

        Variable var;
        getline(file, var.name);
        getline(file, line);
        var.axiom_layer = std::stoi(line);
        getline(file, line);
        var.range = stoi(line);

        for (int j = 0; j < var.range; j++) {
            getline(file, line);
            var.sym_names.push_back(line);
        }

        getline(file, line);
        assert(line == "end_variable");
        vars.push_back(var);
    }

    assert(n_vars == vars.size());
    return vars;
}

Fact PlanningTaskParser::parse_fact(std::string line){
    Fact fact;
    std::istringstream iss(line);
    std::string tok;

    iss >> tok;
    fact.var_idx = std::stoi(tok);

    iss >> tok;
    fact.var_val = std::stoi(tok);

    return fact;
}

std::vector<MutexGroup> PlanningTaskParser::get_facts(std::ifstream &file) {
    std::vector<MutexGroup> mutexes;
    std::string line;

    // number of mutex groups
    getline(file, line);
    int n_mutex = stoi(line);

    for (int i = 0; i < n_mutex; i++) {
        getline(file, line);
        assert(line == "begin_mutex_group");

        MutexGroup mutex;
        getline(file, line);
        mutex.n_facts = std::stoi(line);

        for (int i = 0; i < mutex.n_facts; i++) {
            getline(file, line);
            mutex.facts.push_back(parse_fact(line));
        }

        getline(file, line);
        assert(line == "end_mutex_group");
        mutexes.push_back(mutex);
    }

    assert(n_mutex == mutexes.size());
    return mutexes;
}

std::vector<int> PlanningTaskParser::get_initial_state(std::ifstream &file, int n_vars) {
    std::vector<int> initial_state;
    std::string line;
    getline(file, line);

    assert(line == "begin_state");

    for (int i = 0; i < n_vars; i++) {
        getline(file, line);
        int var = std::stoi(line);
        initial_state.push_back(var);
    }

    getline(file, line);
    assert(line == "end_state");

    return initial_state;
}

std::vector<Fact> PlanningTaskParser::get_goal(std::ifstream &file) {
    std::vector<Fact> goal_state;
    std::string line;
    getline(file, line);

    assert(line == "begin_goal");

    getline(file, line);
    int n_goals = std::stoi(line);

    for (int i = 0; i < n_goals; i++) {
        getline(file, line);
        goal_state.push_back(parse_fact(line));
    }

    getline(file, line);
    assert(line == "end_goal");

    assert(n_goals == goal_state.size());
    return goal_state;
}

std::vector<Action> PlanningTaskParser::get_actions(std::ifstream &file) {
    std::vector<Action> actions;
    std::string line;

    getline(file, line);
    int n_actions = std::stoi(line);

    for (int i = 0; i < n_actions; i++) {
        getline(file, line);
        assert(line == "begin_operator");

        Action action;
        getline(file, action.name);

        getline(file, line);
        action.n_preconds = stoi(line);

        for (int i = 0; i < action.n_preconds; i++) {
            getline(file, line);
            action.preconds.push_back(parse_fact(line));
        }

        getline(file, line);
        action.n_effects = stoi(line);

        for (int i = 0; i < action.n_effects; i++) {
            getline(file, line);
            Effect effect;

            std::istringstream iss(line);
            std::string tok;

            iss >> tok;
            effect.n_effect_conds = std::stoi(tok);

            for (int i = 0; i < effect.n_effect_conds; i++) {
                Fact fact;

                iss >> tok;
                fact.var_idx = std::stoi(tok);

                iss >> tok;
                fact.var_val = std::stoi(tok);

                effect.effect_conds.push_back(fact);
            }

            iss >> tok;
            effect.var_affected = std::stoi(tok);

            iss >> tok;
            effect.from_value = std::stoi(tok);

            iss >> tok;
            effect.to_value = std::stoi(tok);

            action.effects.push_back(effect);
        }

        getline(file, line);
        action.cost = std::stoi(line);
        action.is_used = false;
        action.h_cost = -1; // not valid
        actions.push_back(action);

        getline(file, line);
        assert(line == "end_operator");
    }

    assert(n_actions == actions.size());
    return actions;
}

std::vector<Axiom> PlanningTaskParser::get_axioms(std::ifstream &file) {
    std::vector<Axiom> axioms;
    std::string line;
    getline(file, line);
    int n_axioms = stoi(line);

    for (int i = 0; i < n_axioms; i++) {
        getline(file, line);
        assert(line == "begin_rule");

        Axiom axiom;
        getline(file, line);
        axiom.n_conds = std::stoi(line);

        for (int j = 0; j < axiom.n_conds; j++) {
            getline(file, line);
            axiom.conds.push_back(parse_fact(line));
        }

        getline(file, line);

        std::istringstream iss(line);
        std::string tok;

        iss >> tok;
        axiom.affected_var = std::stoi(tok);

        iss >> tok;
        axiom.from_value = std::stoi(tok);

        iss >> tok;
        axiom.to_value = std::stoi(tok);

        axioms.push_back(axiom);

        getline(file, line);
        assert(line == "end_rule");
    }

    assert(n_axioms == axioms.size());
    return axioms;
}

PlanningTask PlanningTaskParser::parse_from_file(std::string filename) {
    std::ifstream file (filename);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file");
    }

    assert_version(file);
    int metric = get_metric(file);
    std::vector<Variable> vars = get_variables(file);
    std::vector<MutexGroup> mutexes = get_facts(file);
    std::vector<int> initial_state = get_initial_state(file, vars.size());
    std::vector<Fact> goal_state = get_goal(file);
    std::vector<Action> actions = get_actions(file);
    std::vector<Axiom> axioms = get_axioms(file);

    file.close();

    return PlanningTask(metric, vars.size(), vars, mutexes.size(), mutexes,
        initial_state, goal_state.size(), goal_state, actions.size(),
        actions, axioms.size(), axioms);
}
