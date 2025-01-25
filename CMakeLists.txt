cmake_minimum_required (VERSION 3.2 FATAL_ERROR)
project(ai-planning)

add_executable(main
	main.cpp
	src/planning_task.cpp
	src/planning_task_utils.cpp
	src/planning_task_parser.cpp
)

# Find OpenMP
find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
    target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found and linked!")
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()
