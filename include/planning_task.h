#ifndef PLANNING_TASK_H
#define PLANNING_TASK_H

#include <fstream>
#include <string>

class PlanningTask {
public:
    int parse_from_file(std::string filenamme);
private:
    int metric; // 0 no action costs, 1 action costs
    void assert_version(std::ifstream &file);
    void get_metric(std::ifstream &file);
};

#endif
