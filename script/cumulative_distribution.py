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
output_base_dir = '../out_full/'
alg1_dir = output_base_dir + 'random/'
alg2_dir = output_base_dir + 'greedy/'
alg3_dir = output_base_dir + 'hmax_rec/'
alg4_dir = '../out_backprop/backprop/'

# Collect test instances
test_instances = [
    filename for filename in listdir('../DeletefreeSAS')
]

primal_gaps = [[] for _ in range(4)]
n_iter = 1

for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Get matching files for each algorithm
    alg1_files = [file for file in listdir(alg1_dir) if re.search(instance_base_name, file)]
    alg2_files = [file for file in listdir(alg2_dir) if re.search(instance_base_name, file)]
    alg3_files = [file for file in listdir(alg3_dir) if re.search(instance_base_name, file)]
    alg4_files = [file for file in listdir(alg4_dir) if re.search(instance_base_name, file)]

    # Extract costs
    instance_costs = [
        extract_instance_costs(alg1_files, alg1_dir),
        extract_instance_costs(alg2_files, alg2_dir),
        extract_instance_costs(alg3_files, alg3_dir),
        extract_instance_costs(alg4_files, alg4_dir),
    ]

    # Find best known cost
    with open("best_known.txt", "r") as f:
        for line in f:
            if instance in line:
                best_known = int(line.strip()[-1])

    # Compute primal gaps
    for alg_index, sublist in enumerate(instance_costs):
        for cost in sublist:
            primal_gaps[alg_index].append(compute_primal_gap(cost, best_known))

    if n_iter % 100 == 0:
        print(f'{n_iter}/{len(test_instances)}')
    n_iter += 1

# Compute cumulative distributions for each algorithm
alg1_points = get_alg_points(primal_gaps[0])
alg2_points = get_alg_points(primal_gaps[1])
alg3_points = get_alg_points(primal_gaps[2])
alg4_points = get_alg_points(primal_gaps[3])

print('alg1_points = ', alg1_points)
print('alg2_points = ', alg2_points)
print('alg3_points = ', alg3_points)
print('alg4_points = ', alg4_points)


# Define x-axis values (thresholds from 0.0 to 1.0)
x_values = np.arange(0.0, 1.1, 0.1)

# Plot results
plt.figure(figsize=(8, 6))
plt.plot(x_values, alg1_points, marker='o', linestyle='-', label='random')
plt.plot(x_values, alg2_points, marker='s', linestyle='-', label='greedy')
plt.plot(x_values, alg3_points, marker='^', linestyle='-', label='hmax_rec')
plt.plot(x_values, alg4_points, marker='d', linestyle='-', label='backprop')

# Labels and Title
plt.xlabel("Primal Gap Threshold (0.0 to 1.0)")
plt.ylabel("Fraction of Instances with Gap ≤ Threshold")
plt.title("Cumulative Distribution of Primal Gaps")

# Grid and Legend
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.show()
