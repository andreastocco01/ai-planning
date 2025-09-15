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
                    instance_costs.append(-1)
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
output_base_dir = '../out_set_var/'
alg1_dir = output_base_dir + 'random/'
alg2_dir = output_base_dir + 'greedy/'
alg3_dir = output_base_dir + 'pruning/'
alg4_dir = output_base_dir + 'lookahead/'
alg5_dir = output_base_dir + 'backprop_min/'
alg6_dir = output_base_dir + 'backprop_max/'
alg7_dir = output_base_dir + 'backprop_sum/'
alg8_dir = output_base_dir + 're-apply_backprop_min/'
alg9_dir = output_base_dir + 'backprop_min_ucs/'

# Collect test instances
test_instances = [
    filename for filename in listdir('../DeletefreeSAS')
]

primal_gaps = [[] for _ in range(23)]
n_iter = 1

for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Find best known cost
    with open("best_known_set_var.txt", "r") as f:
        for line in f:
            if instance in line:
                best_known = int(line.strip()[-1])

    # Skip non solved instances
    if best_known == -1:
        n_iter += 1
        continue

    # Get matching files for each algorithm
    alg1_files = [file for file in listdir(alg1_dir) if re.search(instance_base_name, file)]
    alg2_files = [file for file in listdir(alg2_dir) if re.search(instance_base_name, file)]
    alg3_files = [file for file in listdir(alg3_dir) if re.search(instance_base_name, file)]
    alg4_files = [file for file in listdir(alg4_dir) if re.search(instance_base_name, file)]
    alg5_files = [file for file in listdir(alg5_dir) if re.search(instance_base_name, file) and re.search("_47.out", file)]
    alg6_files = [file for file in listdir(alg6_dir) if re.search(instance_base_name, file)]
    alg7_files = [file for file in listdir(alg7_dir) if re.search(instance_base_name, file)]
    alg8_files_03 = [file for file in listdir(alg8_dir) if re.search(instance_base_name, file) and re.search("0.3.out", file)]
    alg8_files_07 = [file for file in listdir(alg8_dir) if re.search(instance_base_name, file) and re.search("0.7.out", file)]
    alg8_files_1 = [file for file in listdir(alg8_dir) if re.search(instance_base_name, file) and re.search("1.out", file)]
    alg9_files_03 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.3.out", file)]
    alg9_files_07 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.7.out", file)]
    alg9_files_1 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("1.out", file)]
    alg10_files_01 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.1.out", file)]
    alg10_files_02 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.2.out", file)]
    alg10_files_03 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.3.out", file)]
    alg10_files_04 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.4.out", file)]
    alg10_files_05 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.5.out", file)]
    alg10_files_06 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.6.out", file)]
    alg10_files_07 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.7.out", file)]
    alg10_files_08 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.8.out", file)]
    alg10_files_09 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.9.out", file)]
    alg10_files_1 = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file) and re.search("0.9_1.out", file)]

    # Extract costs
    instance_costs = [
        extract_instance_costs(alg1_files, alg1_dir),
        extract_instance_costs(alg2_files, alg2_dir),
        extract_instance_costs(alg3_files, alg3_dir),
        extract_instance_costs(alg4_files, alg4_dir),
        extract_instance_costs(alg5_files, alg5_dir),
        extract_instance_costs(alg6_files, alg6_dir),
        extract_instance_costs(alg7_files, alg7_dir),
        extract_instance_costs(alg8_files_03, alg8_dir),
        extract_instance_costs(alg8_files_07, alg8_dir),
        extract_instance_costs(alg8_files_1, alg8_dir),
        extract_instance_costs(alg9_files_03, alg9_dir),
        extract_instance_costs(alg9_files_07, alg9_dir),
        extract_instance_costs(alg9_files_1, alg9_dir),
        extract_instance_costs(alg10_files_01, alg9_dir),
        extract_instance_costs(alg10_files_02, alg9_dir),
        extract_instance_costs(alg10_files_03, alg9_dir),
        extract_instance_costs(alg10_files_04, alg9_dir),
        extract_instance_costs(alg10_files_05, alg9_dir),
        extract_instance_costs(alg10_files_06, alg9_dir),
        extract_instance_costs(alg10_files_07, alg9_dir),
        extract_instance_costs(alg10_files_08, alg9_dir),
        extract_instance_costs(alg10_files_09, alg9_dir),
        extract_instance_costs(alg10_files_1, alg9_dir),
    ]

    # keep only the instances solved by both
    '''if instance_costs[0].count(-1) == len(instance_costs[0]) or instance_costs[1].count(-1) == len(instance_costs[1]) or instance_costs[2].count(-1) == len(instance_costs[2]):
        n_iter += 1
        continue'''

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
alg5_points = get_alg_points(primal_gaps[4])
alg6_points = get_alg_points(primal_gaps[5])
alg7_points = get_alg_points(primal_gaps[6])
alg8_files_03_points = get_alg_points(primal_gaps[7])
alg8_files_07_points = get_alg_points(primal_gaps[8])
alg8_files_1_points = get_alg_points(primal_gaps[9])
alg9_files_03_points = get_alg_points(primal_gaps[10])
alg9_files_07_points = get_alg_points(primal_gaps[11])
alg9_files_1_points = get_alg_points(primal_gaps[12])
alg10_files_01_points = get_alg_points(primal_gaps[13])
alg10_files_02_points = get_alg_points(primal_gaps[14])
alg10_files_03_points = get_alg_points(primal_gaps[15])
alg10_files_04_points = get_alg_points(primal_gaps[16])
alg10_files_05_points = get_alg_points(primal_gaps[17])
alg10_files_06_points = get_alg_points(primal_gaps[18])
alg10_files_07_points = get_alg_points(primal_gaps[19])
alg10_files_08_points = get_alg_points(primal_gaps[20])
alg10_files_09_points = get_alg_points(primal_gaps[21])
alg10_files_1_points = get_alg_points(primal_gaps[22])

print('alg1_points = ', alg1_points)
print('alg2_points = ', alg2_points)
print('alg3_points = ', alg3_points)
print('alg4_points = ', alg4_points)
print('alg5_points = ', alg5_points)
print('alg6_points = ', alg6_points)
print('alg7_points = ', alg7_points)
print('alg8_points = ', alg8_files_03_points)
print('alg9_points = ', alg8_files_07_points)
print('alg10_points = ', alg8_files_1_points)
print('alg11_points = ', alg9_files_03_points)
print('alg12_points = ', alg9_files_07_points)
print('alg13_points = ', alg9_files_1_points)
print('alg14_points = ', alg10_files_01_points)
print('alg15_points = ', alg10_files_02_points)
print('alg16_points = ', alg10_files_03_points)
print('alg17_points = ', alg10_files_04_points)
print('alg18_points = ', alg10_files_05_points)
print('alg19_points = ', alg10_files_06_points)
print('alg20_points = ', alg10_files_07_points)
print('alg21_points = ', alg10_files_08_points)
print('alg22_points = ', alg10_files_09_points)
print('alg23_points = ', alg10_files_1_points)


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
