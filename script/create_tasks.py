from os import listdir

files = [file for file in listdir('../DeletefreeSAS/')]
algs = ['7', '8']
# seeds = range(42, 52)
intervals = [(0, 0.3), (0.3, 0.7), (0.7, 1)]
intervals2 = [(0, 0.1), (0.1, 0.2), (0.2, 0.3), (0.3, 0.4), (0.4, 0.5), (0.5, 0.6), (0.6, 0.7), (0.7, 0.8), (0.8, 0.9), (0.9, 1)]

with open('tasks.txt', 'w') as out:
    for file in files:
        for alg in algs:
            #for seed in seeds:
            for interval in intervals:
                out.write('{} {} {} {} {}\n'.format(file, alg, 47, interval[0], interval[1]))
                # out.write('{} {} {}\n'.format(file, alg, seed))

with open('tasks.txt', 'a') as out:
    for file in files:
        for interval in intervals2:
            out.write('{} {} {} {} {}\n'.format(file, 8, 47, interval[0], interval[1]))
