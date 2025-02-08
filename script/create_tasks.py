from os import listdir

files = [file for file in listdir('../DeletefreeSAS/')]
algs = ['0', '1', '2', '3']
seeds = range(42, 52)

with open('tasks.txt', 'w') as out:
    for file in files:
        for alg in algs:
            for seed in seeds:
                out.write('{} {} {}\n'.format(file, alg, seed))
