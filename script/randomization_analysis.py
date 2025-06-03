from os import listdir
import re
import numpy as np
import matplotlib.pyplot as plt

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

output_base_dir = '../out/'
alg_dir = output_base_dir + 'random/'

test_instances = [
    filename for filename in listdir('../DeletefreeSAS')
]

n_iter = 0
min_points = []
avg_points = []
for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Get matching files for each algorithm
    alg_files = [file for file in listdir(alg_dir) if re.search(instance_base_name, file)]

    # Extract costs
    instance_costs = [
        extract_instance_costs(alg_files, alg_dir),
    ]

    # Find best known cost (excluding -1 values)
    valid_costs = [cost for sublist in instance_costs for cost in sublist if cost != -1]
    if valid_costs:
        best_known = min(valid_costs)
        avg = sum(valid_costs) / len(valid_costs)

        min_points.append(best_known)
        avg_points.append(avg)
    else:
        n_iter += 1
        continue

    if n_iter % 100 == 0:
        print(f'{n_iter}/{len(test_instances)}')
    n_iter += 1

print(f'min points: {min_points}')
print(f'avg points: {avg_points}')

# Define x-axis values (thresholds from 0.0 to 1.0)
x_values = range(1, len(min_points))

# Plot results
plt.figure(figsize=(8, 6))
plt.plot(x_values, min_points, marker='o', linestyle='-', label='min')
plt.plot(x_values, avg_points, marker='s', linestyle='-', label='avg')

# Labels and Title
plt.xlabel("Instance")
plt.title("Randomization analysis")

# Grid and Legend
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.show()
