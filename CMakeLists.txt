cmake_minimum_required (VERSION 3.2 FATAL_ERROR)
project(ai-planning)

add_executable(main
	main.cpp
	src/planning_task.cpp
	src/planning_task_utils.cpp
	src/planning_task_parser.cpp
)
