from os import listdir
import re

def extract_instance_costs(file_list, directory):
    """ Extracts costs from output files. Returns -1 if no valid cost is found. """
    instance_costs = []
    for filename in file_list:
        with open(directory + filename, 'r') as file:
            print(filename)
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

f = open("best_known_set_var.txt", "w")
n_iter = 1

for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Get matching files for each algorithm
    alg1_files = [file for file in listdir(alg1_dir) if re.search(instance_base_name, file)]
    alg2_files = [file for file in listdir(alg2_dir) if re.search(instance_base_name, file)]
    alg3_files = [file for file in listdir(alg3_dir) if re.search(instance_base_name, file)]
    alg4_files = [file for file in listdir(alg4_dir) if re.search(instance_base_name, file)]
    alg5_files = [file for file in listdir(alg5_dir) if re.search(instance_base_name, file)]
    alg6_files = [file for file in listdir(alg6_dir) if re.search(instance_base_name, file)]
    alg7_files = [file for file in listdir(alg7_dir) if re.search(instance_base_name, file)]
    alg8_files = [file for file in listdir(alg8_dir) if re.search(instance_base_name, file)]
    alg9_files = [file for file in listdir(alg9_dir) if re.search(instance_base_name, file)]

    # Extract costs
    instance_costs = [
        extract_instance_costs(alg1_files, alg1_dir),
        extract_instance_costs(alg2_files, alg2_dir),
        extract_instance_costs(alg3_files, alg3_dir),
        extract_instance_costs(alg4_files, alg4_dir),
        extract_instance_costs(alg5_files, alg5_dir),
        extract_instance_costs(alg6_files, alg6_dir),
        extract_instance_costs(alg7_files, alg7_dir),
        extract_instance_costs(alg8_files, alg8_dir),
        extract_instance_costs(alg9_files, alg9_dir),
    ]

    # Find best known cost (excluding -1 values)
    valid_costs = [cost for sublist in instance_costs for cost in sublist if cost != -1]
    best_known = min(valid_costs) if valid_costs else -1  # If no valid solution, keep -1

    f.write(f'Instance: {instance}, Best known cost: {best_known}\n')
    if n_iter % 100 == 0:
        print(f'{n_iter}/{len(test_instances)}')
    n_iter += 1

f.close()
