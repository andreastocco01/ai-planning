from os import listdir
import re
import matplotlib.pyplot as plt
import numpy as np

def extract_instance_costs(file_list, directory):
    """ Extracts costs from output files. Returns -1 if no valid cost is found. """
    instance_costs = []
    for filename in file_list:
        with open(directory + filename, 'r') as file:
            content = file.read()
            if "Solution found!" in content:
                file.seek(0)
                lines = file.readlines()
                try:
                    instance_costs.append(int(lines[-1].split(' ')[1]))
                except:
                    print(directory + filename)
            elif "Solution does not exist!" in content:
                instance_costs.append(0)
            else:
                instance_costs.append(-1)
    return instance_costs

def compute_primal_gap(cost, best_known):
    """ Computes the primal gap for a given cost. """
    if cost == -1:  # No solution found
        return 1
    if best_known == 0 and cost == 0:
        return 0
    if best_known * cost < 0:  # Opposite signs
        return 1
    return abs(best_known - cost) / max(abs(best_known), abs(cost))

def get_alg_points(alg_primal_gaps):
    """ Computes the percentage of instances with a primal gap below a threshold. """
    res = []
    for threshold in np.arange(0.0, 1.1, 0.1):
        count = sum(1 for gap in alg_primal_gaps if gap <= threshold)
        res.append(count / len(alg_primal_gaps) if alg_primal_gaps else 0)
    return res


# Define directories
output_base_dir = '../out'
random_alg_dir = output_base_dir + '/random/'
greedy_alg_dir = output_base_dir + '/greedy/'
hadd_alg_dir = output_base_dir + '/hadd/'
hmax_alg_dir = output_base_dir + '/hmax/'

# Collect test instances
test_instances = [
    filename for filename in listdir('../DeletefreeSAS')
]

primal_gaps = [[] for _ in range(4)]  # [0]: random, [1]: greedy, [2]: hadd, [3]: hmax

for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Get matching files for each algorithm
    random_alg_files = [file for file in listdir(random_alg_dir) if re.search(instance_base_name, file)]
    greedy_alg_files = [file for file in listdir(greedy_alg_dir) if re.search(instance_base_name, file)]
    hadd_alg_files = [file for file in listdir(hadd_alg_dir) if re.search(instance_base_name, file)]
    hmax_alg_files = [file for file in listdir(hmax_alg_dir) if re.search(instance_base_name, file)]

    # Extract costs
    instance_costs = [
        extract_instance_costs(random_alg_files, random_alg_dir),
        extract_instance_costs(greedy_alg_files, greedy_alg_dir),
        extract_instance_costs(hadd_alg_files, hadd_alg_dir),
        extract_instance_costs(hmax_alg_files, hmax_alg_dir),
    ]

    # Find best known cost (excluding -1 values)
    valid_costs = [cost for sublist in instance_costs for cost in sublist if cost != -1]
    best_known = min(valid_costs) if valid_costs else -1  # If no valid solution, keep -1

    # print(f'Instance: {instance}, Best known cost: {best_known}')

    # Compute primal gaps
    for alg_index, sublist in enumerate(instance_costs):
        for cost in sublist:
            primal_gaps[alg_index].append(compute_primal_gap(cost, best_known))

# Compute cumulative distributions for each algorithm
random_points = get_alg_points(primal_gaps[0])
greedy_points = get_alg_points(primal_gaps[1])
hadd_points = get_alg_points(primal_gaps[2])
hmax_points = get_alg_points(primal_gaps[3])

print('random_points = ', random_points)
print('greedy_points = ', greedy_points)
print('hadd_points = ', hadd_points)
print('hmax_points = ', hmax_points)

# Define x-axis values (thresholds from 0.0 to 1.0)
x_values = np.arange(0.0, 1.1, 0.1)

# Plot results
plt.figure(figsize=(8, 6))
plt.plot(x_values, random_points, marker='o', linestyle='-', label='Random')
plt.plot(x_values, greedy_points, marker='s', linestyle='-', label='Greedy')
plt.plot(x_values, hadd_points, marker='^', linestyle='-', label='HADD')
plt.plot(x_values, hmax_points, marker='d', linestyle='-', label='HMAX')

# Labels and Title
plt.xlabel("Primal Gap Threshold (0.0 to 1.0)")
plt.ylabel("Fraction of Instances with Gap ≤ Threshold")
plt.title("Cumulative Distribution of Primal Gaps")

# Grid and Legend
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.show()
