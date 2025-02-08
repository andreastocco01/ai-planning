from os import listdir
import re

def extract_instance_costs(file_list, dir):
    instance_costs = []
    for filename in file_list:
        with open(dir + filename, 'r') as file:
            lines = file.readlines()
            if re.match('Cost: ', lines[-1]):
                instance_costs.append(int(lines[-1].strip().split(' ')[1]))
            else:
                instance_costs.append(-1)
    return instance_costs

def compute_primal_gap(cost, best_known):
    if cost == -1:  # No solution found
        return 1
    if best_known == 0 and cost == 0:
        return 0
    if best_known * cost < 0:  # Opposite signs
        return 1
    return abs(best_known - cost) / max(abs(best_known), abs(cost))


output_base_dir = '../out'

random_alg_dir = output_base_dir + '/random/'
greedy_alg_dir = output_base_dir + '/greedy/'
hadd_alg_dir = output_base_dir + '/hadd/'
hmax_alg_dir = output_base_dir + '/hmax/'

test_instances = [
    filename for filename in listdir('../DeletefreeSAS')
    if filename == 'agricola-sat18-strips-p12_deletefree.sas'
]

primal_gaps = []

for instance in test_instances:
    instance_base_name = instance.split('.')[0]  # Remove .sas extension

    random_alg_files = [
        file for file in listdir(random_alg_dir)
        if re.match(instance_base_name, file)
    ]
    greedy_alg_files = [
        file for file in listdir(greedy_alg_dir)
        if re.match(instance_base_name, file)
    ]
    hadd_alg_files = [
        file for file in listdir(hadd_alg_dir)
        if re.match(instance_base_name, file)
    ]
    hmax_alg_files = [
        file for file in listdir(hmax_alg_dir)
        if re.match(instance_base_name, file)
    ]

    instance_costs = []
    instance_costs.append(extract_instance_costs(random_alg_files, random_alg_dir))
    instance_costs.append(extract_instance_costs(greedy_alg_files, greedy_alg_dir))
    instance_costs.append(extract_instance_costs(hadd_alg_files, hadd_alg_dir))
    instance_costs.append(extract_instance_costs(hmax_alg_files, hmax_alg_dir))

    best_known = min([elem for sub in instance_costs for elem in sub])
    print('Best known: ', best_known)

    for sub in instance_costs:
        for cost in sub:
            primal_gaps.append(compute_primal_gap(cost, best_known))

print(primal_gaps)
