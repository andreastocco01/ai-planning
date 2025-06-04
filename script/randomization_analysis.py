from os import listdir
import re
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

n_iter = 1
min_points = []
avg_points = []
for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    # Get matching files for each algorithm
    alg_files = [file for file in listdir(alg_dir) if re.search(instance_base_name, file)]

    # Extract costs
    instance_costs = extract_instance_costs(alg_files, alg_dir)

    # Find best known cost (excluding -1 values)
    valid_costs = [cost for cost in instance_costs if cost != -1]
    if valid_costs:
        best_known = min(valid_costs)
        avg = sum(valid_costs) / len(valid_costs)

        min_points.append(best_known)
        avg_points.append(avg)

    if n_iter % 100 == 0:
        print(f'{n_iter}/{len(test_instances)}')
    n_iter += 1

print("min_points = ", min_points)
print("avg_points = ", avg_points)

plt.figure(figsize=(10, 6))
plt.scatter(min_points, avg_points, alpha=0.5)
plt.plot([min(min_points), max(min_points)], [min(min_points), max(min_points)], 'r--', label='y = x')

plt.xscale('log')
plt.yscale('log')

plt.xlabel('Best Cost (min across seeds)')
plt.ylabel('Average Cost (avg across seeds)')
plt.title('Best vs Average Cost (Log Scale)')
plt.legend()
plt.grid(True, which='both', ls='--')
plt.tight_layout()
plt.show()
