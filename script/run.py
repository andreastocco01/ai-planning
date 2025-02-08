import subprocess

working_dir = "../build"

# List of file paths to use as the --from-file parameter
from_file_paths = [
    "../DeletefreeSAS/sokoban-sat08-strips-p03_deletefree.sas",
    "../DeletefreeSAS/sokoban-sat08-strips-p04_deletefree.sas",
    "../DeletefreeSAS/sokoban-sat08-strips-p05_deletefree.sas",
]

alg = 3
seed = 0
time_limit = 60

# Base command
base_command = "make && ./main --alg {} --from-file {} --seed {} --time-limit {} --debug 0 > {}"

# Iterate over the file paths and execute the command for each
for file_path in from_file_paths:
    # Output file name (you can adjust this logic as needed)
    output_file = file_path.split('/')[-1].replace('.sas', '_output.txt')
    output_file = "../out/" + output_file

    # Construct the full command
    command = base_command.format(alg, file_path, seed, time_limit, output_file)

    print(f"Executing: {command}")

    # Execute the command
    try:
        subprocess.run(command, shell=True, check=True, cwd=working_dir)
        print(f"Completed: Output written to {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Command failed with error: {e}")
