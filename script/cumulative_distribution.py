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

# NO SET VAR
# random
alg1_points =  [0.09874355670103092, 0.09916237113402061, 0.10293170103092783, 0.10480025773195877, 0.10821520618556701, 0.11591494845360825, 0.11936211340206186, 0.1440721649484536, 0.1925257731958763, 0.2592139175257732, 1.0]
# greedy
alg2_points =  [0.15618556701030928, 0.15660438144329897, 0.16427190721649484, 0.1702319587628866, 0.17883376288659794, 0.1858569587628866, 0.18875644329896907, 0.19913015463917524, 0.21894329896907216, 0.28366623711340205, 1.0]
# pruning
alg3_points =  [0.23824097938144329, 0.23952963917525774, 0.2495811855670103, 0.2569587628865979, 0.26485180412371134, 0.27239046391752575, 0.2819265463917526, 0.2986791237113402, 0.33530927835051544, 0.4204896907216495, 1.0]
# lookahead
alg4_points =  [0.22583762886597938, 0.226965206185567, 0.23685567010309277, 0.24339561855670103, 0.25161082474226804, 0.2584085051546392, 0.26878221649484535, 0.28563144329896906, 0.32023195876288657, 0.4009342783505155, 1.0]
# backprop min
alg5_points =  [0.2526417525773196, 0.2533827319587629, 0.2645618556701031, 0.2725837628865979, 0.28079896907216495, 0.28704896907216493, 0.3038337628865979, 0.33170103092783504, 0.38460051546391755, 0.5002899484536083, 1.0]
# backprop_max
alg6_points =  [0.23521262886597938, 0.23611469072164948, 0.24445876288659793, 0.25289948453608246, 0.2621134020618557, 0.2692332474226804, 0.2818943298969072, 0.3043492268041237, 0.3541237113402062, 0.4632731958762887, 1.0]
# backprop sum
alg7_points =  [0.23460051546391752, 0.23530927835051546, 0.2434922680412371, 0.25061211340206185, 0.2593105670103093, 0.268395618556701, 0.2802512886597938, 0.3009020618556701, 0.34445876288659794, 0.45267396907216495, 1.0]
# re apply backprop min
alg8_points =  [0.25315721649484535, 0.2540270618556701, 0.26565721649484536, 0.27248711340206183, 0.2805734536082474, 0.28737113402061853, 0.3050579896907217, 0.3324742268041237, 0.38646907216494847, 0.5054768041237113, 1.0]
alg9_points =  [0.254284793814433, 0.2551868556701031, 0.2681056701030928, 0.27496778350515466, 0.28253865979381443, 0.2887242268041237, 0.3059278350515464, 0.33373067010309276, 0.38830541237113403, 0.5057023195876289, 1.0]
alg10_points =  [0.2552512886597938, 0.2560567010309278, 0.26788015463917525, 0.27554768041237115, 0.28266752577319587, 0.28775773195876286, 0.3045103092783505, 0.33308634020618555, 0.3867590206185567, 0.5033505154639175, 1.0]
# backprop min ucs (0-0.3, 0.3-0.7, 0.7-1)
alg11_points =  [0.2506443298969072, 0.25128865979381443, 0.26417525773195877, 0.2696520618556701, 0.2818943298969072, 0.28704896907216493, 0.30476804123711343, 0.3324742268041237, 0.3872422680412371, 0.5028994845360825, 1.0]
alg12_points =  [0.2545103092783505, 0.2554768041237113, 0.2667525773195876, 0.27384020618556704, 0.2847938144329897, 0.28930412371134023, 0.30541237113402064, 0.3321520618556701, 0.38756443298969073, 0.5022551546391752, 1.0]
alg13_points =  [0.25579896907216493, 0.25676546391752575, 0.26868556701030927, 0.27802835051546393, 0.2851159793814433, 0.28930412371134023, 0.30444587628865977, 0.3321520618556701, 0.38853092783505155, 0.5022551546391752, 1.0]
# backprop min ucs (0-0.1, 0.1-0.2, ..., 0.9-1)
alg14_points =  [0.2509664948453608, 0.25161082474226804, 0.2651417525773196, 0.2702963917525773, 0.28253865979381443, 0.28865979381443296, 0.30573453608247425, 0.3327963917525773, 0.3872422680412371, 0.5032216494845361, 1.0]
alg15_points =  [0.2506443298969072, 0.25161082474226804, 0.264819587628866, 0.2702963917525773, 0.28253865979381443, 0.28865979381443296, 0.30573453608247425, 0.3337628865979381, 0.38885309278350516, 0.5035438144329897, 1.0]
alg16_points =  [0.25128865979381443, 0.25225515463917525, 0.264819587628866, 0.2702963917525773, 0.28286082474226804, 0.28865979381443296, 0.30605670103092786, 0.3337628865979381, 0.3894974226804124, 0.5038659793814433, 1.0]
alg17_points =  [0.2509664948453608, 0.25161082474226804, 0.264819587628866, 0.27158505154639173, 0.28286082474226804, 0.28865979381443296, 0.30670103092783507, 0.33408505154639173, 0.3894974226804124, 0.5041881443298969, 1.0]
alg18_points =  [0.2509664948453608, 0.25193298969072164, 0.2657860824742268, 0.2702963917525773, 0.28318298969072164, 0.28865979381443296, 0.30573453608247425, 0.3331185567010309, 0.3901417525773196, 0.5041881443298969, 1.0]
alg19_points =  [0.2506443298969072, 0.25161082474226804, 0.264819587628866, 0.27190721649484534, 0.28318298969072164, 0.28962628865979384, 0.30573453608247425, 0.3327963917525773, 0.38853092783505155, 0.5045103092783505, 1.0]
alg20_points =  [0.25161082474226804, 0.25257731958762886, 0.264819587628866, 0.2706185567010309, 0.28286082474226804, 0.28962628865979384, 0.30637886597938147, 0.3334407216494845, 0.38885309278350516, 0.5054768041237113, 1.0]
alg21_points =  [0.2509664948453608, 0.25193298969072164, 0.2651417525773196, 0.27222938144329895, 0.28318298969072164, 0.28930412371134023, 0.30605670103092786, 0.3334407216494845, 0.3894974226804124, 0.5048324742268041, 1.0]
alg22_points =  [0.25128865979381443, 0.25225515463917525, 0.2644974226804124, 0.2702963917525773, 0.28286082474226804, 0.29027061855670105, 0.30670103092783507, 0.3334407216494845, 0.38917525773195877, 0.5045103092783505, 1.0]
alg23_points =  [0.2506443298969072, 0.25161082474226804, 0.2651417525773196, 0.27416237113402064, 0.28414948453608246, 0.28865979381443296, 0.30573453608247425, 0.3331185567010309, 0.38853092783505155, 0.5032216494845361, 1.0]

# instances solved by all of them
#alg3_points =  [0.3091685393258427, 0.31096629213483146, 0.3249887640449438, 0.3352808988764045, 0.34629213483146065, 0.35613483146067415, 0.36943820224719104, 0.3927191011235955, 0.44310112359550563, 0.5547415730337079, 1.0]
#alg4_points =  [0.3150561797752809, 0.31662921348314604, 0.3304269662921348, 0.3395505617977528, 0.3510112359550562, 0.3604943820224719, 0.37496629213483146, 0.39847191011235955, 0.4467415730337079, 0.5593258426966292, 1.0]
#alg5_points =  [0.3239101123595506, 0.3249438202247191, 0.34031460674157304, 0.3510561797752809, 0.3621123595505618, 0.37024719101123593, 0.39065168539325845, 0.426247191011236, 0.48921348314606744, 0.6172134831460674, 1.0]

# backprop_min seed 47
alg5_points =  [0.25193298969072164, 0.25289948453608246, 0.2661082474226804, 0.27416237113402064, 0.27996134020618557, 0.28801546391752575, 0.30637886597938147, 0.3318298969072165, 0.38369845360824745, 0.4993556701030928, 1.0]

# SET VAR
# random
alg1_points =  [0.0710738471460819, 0.07149306675266043, 0.07526604321186714, 0.07710415994840374, 0.08045791680103193, 0.09016446307642696, 0.0943244114801677, 0.11696227023540794, 0.16984843598839083, 0.245501451144792, 1.0]
# greedy
alg2_points =  [0.12850693324733956, 0.1289261528539181, 0.13679458239277653, 0.14295388584327637, 0.15159625927120285, 0.1583037729764592, 0.16127055788455336, 0.17165430506288293, 0.19377620122541117, 0.27194453402128343, 1.0]
# pruning
alg3_points =  [0.19590454692034828, 0.19729119638826184, 0.20851338277974846, 0.21528539180909384, 0.22415349887133182, 0.2310867462108997, 0.24111576910673976, 0.2600773943889068, 0.2990648178007094, 0.39858110287004195, 1.0]
# lookahead
alg4_points =  [0.19350988677784586, 0.19476791071255767, 0.2053804715976904, 0.21176736234315022, 0.22031547369439697, 0.22557336860101287, 0.234766620431599, 0.2539595496919454, 0.2891519628399084, 0.3752782168317151, 1.0]
# backprop min
alg5_points =  [0.21974193548387097, 0.22054838709677418, 0.23241935483870968, 0.2395483870967742, 0.24738709677419354, 0.25290322580645164, 0.2695161290322581, 0.2980967741935484, 0.35429032258064513, 0.47680645161290325, 1.0]
# backprop max
alg6_points =  [0.19525806451612904, 0.19622580645161292, 0.20596774193548387, 0.21419354838709678, 0.2235483870967742, 0.2318709677419355, 0.24448387096774193, 0.26903225806451614, 0.3221612903225806, 0.4386129032258064, 1.0]
# backprop sum
alg7_points =  [0.20180645161290323, 0.2025483870967742, 0.21180645161290323, 0.21870967741935485, 0.22732258064516128, 0.23525806451612905, 0.24603225806451612, 0.26825806451612905, 0.3133548387096774, 0.4295806451612903, 1.0]
# re apply backprop min
alg8_points =  [0.2184278350515464, 0.2190721649484536, 0.23099226804123713, 0.23582474226804123, 0.24806701030927836, 0.25289948453608246, 0.2709407216494845, 0.29896907216494845, 0.35760309278350516, 0.47809278350515466, 1.0]
alg9_points =  [0.22132731958762886, 0.22197164948453607, 0.2338917525773196, 0.24033505154639176, 0.25225515463917525, 0.25579896907216493, 0.27287371134020616, 0.3021907216494845, 0.3588917525773196, 0.48228092783505155, 1.0]
alg10_points =  [0.22164948453608246, 0.22261597938144329, 0.2348582474226804, 0.2419458762886598, 0.25161082474226804, 0.2554768041237113, 0.27158505154639173, 0.29832474226804123, 0.35695876288659795, 0.47744845360824745, 1.0]
# backprop min ucs (0-0.3, 0.3-0.7, 0.7-1)
alg11_points =  [0.2182667525773196, 0.2190721649484536, 0.23067010309278352, 0.23582474226804123, 0.24822809278350516, 0.25273840206185566, 0.2701353092783505, 0.29880798969072164, 0.35663659793814434, 0.47487113402061853, 1.0]
alg12_points =  [0.22036082474226804, 0.22116623711340205, 0.23228092783505155, 0.23840206185567012, 0.2495167525773196, 0.2535438144329897, 0.2698131443298969, 0.29816365979381443, 0.3559922680412371, 0.47535438144329895, 1.0]
alg13_points =  [0.2189647766323024, 0.21982388316151202, 0.23217353951890035, 0.23979810996563575, 0.24924828178694158, 0.25322164948453607, 0.2695446735395189, 0.2977878006872852, 0.35642182130584193, 0.47454896907216493, 1.0]
# backprop min ucs (0-0.1, 0.1-0.2, ..., 0.9-1)
alg14_points =  [0.21746134020618557, 0.21810567010309279, 0.23034793814432988, 0.23582474226804123, 0.24806701030927836, 0.25257731958762886, 0.2699742268041237, 0.297680412371134, 0.3559922680412371, 0.47454896907216493, 1.0]
alg15_points =  [0.21713917525773196, 0.21810567010309279, 0.23034793814432988, 0.23582474226804123, 0.24806701030927836, 0.25257731958762886, 0.2706185567010309, 0.29864690721649484, 0.35663659793814434, 0.47487113402061853, 1.0]
alg16_points =  [0.2182667525773196, 0.2190721649484536, 0.23067010309278352, 0.23582474226804123, 0.24822809278350516, 0.25273840206185566, 0.2701353092783505, 0.29880798969072164, 0.35663659793814434, 0.47487113402061853, 1.0]
alg17_points =  [0.21746134020618557, 0.21810567010309279, 0.23034793814432988, 0.23711340206185566, 0.24838917525773196, 0.25257731958762886, 0.2702963917525773, 0.29864690721649484, 0.35695876288659795, 0.47551546391752575, 1.0]
alg18_points =  [0.21746134020618557, 0.2184278350515464, 0.23099226804123713, 0.23582474226804123, 0.24838917525773196, 0.25257731958762886, 0.2699742268041237, 0.2980025773195876, 0.3556701030927835, 0.47551546391752575, 1.0]
alg19_points =  [0.21713917525773196, 0.21810567010309279, 0.23002577319587628, 0.23743556701030927, 0.24838917525773196, 0.2535438144329897, 0.2696520618556701, 0.2980025773195876, 0.3556701030927835, 0.47583762886597936, 1.0]
alg20_points =  [0.22036082474226804, 0.22116623711340205, 0.23228092783505155, 0.23840206185567012, 0.2495167525773196, 0.2535438144329897, 0.2698131443298969, 0.29816365979381443, 0.3559922680412371, 0.47535438144329895, 1.0]
alg21_points =  [0.21746134020618557, 0.2184278350515464, 0.23067010309278352, 0.23743556701030927, 0.24838917525773196, 0.25289948453608246, 0.2696520618556701, 0.2980025773195876, 0.35663659793814434, 0.47648195876288657, 1.0]
alg22_points =  [0.21778350515463918, 0.21875, 0.23002577319587628, 0.23582474226804123, 0.24838917525773196, 0.2541881443298969, 0.2706185567010309, 0.2980025773195876, 0.3559922680412371, 0.47648195876288657, 1.0]
alg23_points =  [0.21713917525773196, 0.21810567010309279, 0.23067010309278352, 0.24033505154639176, 0.24903350515463918, 0.25257731958762886, 0.2693298969072165, 0.2980025773195876, 0.3553479381443299, 0.47454896907216493, 1.0]

# instances solved by all of them
#alg3_points =  [0.2583294447036864, 0.26033597760149324, 0.27657489500699955, 0.28637424171721887, 0.2992067195520299, 0.3088660755949603, 0.32333177788147455, 0.3507232851143257, 0.4059729351376575, 0.5379841343910406, 1.0]
#alg4_points =  [0.27993467102193187, 0.2817545496966869, 0.29710685954269717, 0.30634624358376106, 0.31871208586094263, 0.3263182454503033, 0.3396173588427438, 0.3673821745216986, 0.41829211385907605, 0.5428838077461503, 1.0]
#alg5_points =  [0.28810079328044796, 0.2892673821745217, 0.3062995800279981, 0.3160522631824545, 0.3267848810079328, 0.33411105926271584, 0.3551563229118059, 0.39169388707419506, 0.46061595893607093, 0.5952869808679422, 1.0]

# backprop min seed 47
alg5_points =  [0.21741935483870967, 0.21838709677419355, 0.23032258064516128, 0.2361290322580645, 0.24838709677419354, 0.25290322580645164, 0.2696774193548387, 0.29774193548387096, 0.3554838709677419, 0.47483870967741937, 1.0]

# Define x-axis values (thresholds from 0.0 to 1.0)
x_values = np.arange(0.0, 1.1, 0.1)

# Plot results
plt.figure(figsize=(8, 6))
plt.plot(x_values, alg1_points, marker='^', linestyle='-', label='Random', color='blue')
plt.plot(x_values, alg2_points, marker='o', linestyle='-', label='Greedy', color='green')
plt.plot(x_values, alg3_points, marker='s', linestyle='-', label='Greedy + Pruning', color='red')
plt.plot(x_values, alg4_points, marker='D', linestyle='-', label='Hmax + Lookahead', color='purple')
plt.plot(x_values, alg5_points, marker='v', linestyle='-', label='Shortest Path', color='orange')
plt.plot(x_values, alg6_points, marker='>', linestyle='-', label='Max-Cost Backpropagation', color='teal')
plt.plot(x_values, alg7_points, marker='<', linestyle='-', label='Sum-Cost Backpropagation', color='brown')
plt.plot(x_values, alg8_points, marker='P', linestyle='-', label='Reapply Shortest Path 0-0.3', color='magenta')
plt.plot(x_values, alg9_points, marker='x', linestyle='-', label='Reapply Shortest Path 0.3-0.7', color='olive')
plt.plot(x_values, alg10_points, marker='*', linestyle='-', label='Reapply Shortest Path 0.7-1', color='cyan')
plt.plot(x_values, alg11_points, marker='h', linestyle='-', label='UCS 0-0.3', color='darkblue')
plt.plot(x_values, alg12_points, marker='H', linestyle='-', label='UCS 0.3-0.7', color='darkgreen')
plt.plot(x_values, alg13_points, marker='+', linestyle='-', label='UCS 0.7-1', color='darkred')
plt.plot(x_values, alg14_points, marker='d', linestyle='-', label='UCS 0-0.1', color='darkorange')
plt.plot(x_values, alg15_points, marker='|', linestyle='-', label='UCS 0.1-0.2', color='navy')
plt.plot(x_values, alg16_points, marker='_', linestyle='-', label='UCS 0.2-0.3', color='forestgreen')
plt.plot(x_values, alg17_points, marker='.', linestyle='-', label='UCS 0.3-0.4', color='maroon')
plt.plot(x_values, alg18_points, marker=',', linestyle='-', label='UCS 0.4-0.5', color='darkviolet')
plt.plot(x_values, alg19_points, marker='1', linestyle='-', label='UCS 0.5-0.6', color='goldenrod')
plt.plot(x_values, alg20_points, marker='2', linestyle='-', label='UCS 0.6-0.7', color='slateblue')
plt.plot(x_values, alg21_points, marker='3', linestyle='-', label='UCS 0.7-0.8', color='darkcyan')
plt.plot(x_values, alg22_points, marker='4', linestyle='-', label='UCS 0.8-0.9', color='crimson')
plt.plot(x_values, alg23_points, marker='X', linestyle='-', label='UCS 0.9-1', color='black')

# Labels and Title
plt.xlabel("Primal Gap Threshold (0.0 to 1.0)", fontsize=16)
plt.ylabel("Fraction of Instances with Gap ≤ Threshold", fontsize=16)
plt.title("Cumulative Distribution of Primal Gaps", fontsize=18)

# Grid and Legend
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend(fontsize=14)

plt.xticks(fontsize=14)
plt.yticks(fontsize=14)

plt.tight_layout()
plt.show()
