import os
import re

def find_missing_instances(instances_folder, output_folder):
    # Get all instance file names (without extensions)
    instance_files = {f.rsplit(".", 1)[0]: os.path.getsize(os.path.join(instances_folder, f))
                      for f in os.listdir(instances_folder) if f.endswith(".sas")}
    
    # Extract instance names from output files
    output_files = set()
    pattern = re.compile(r"^(.*)_\w+_\d+\.out$")
    
    for f in os.listdir(output_folder):
        match = pattern.match(f)
        if match:
            output_files.add(match.group(1))
    
    # Find instances without corresponding output files
    missing_instances = {name: size for name, size in instance_files.items() if name not in output_files}
    
    # Sort missing instances by size in descending order
    sorted_missing_instances = sorted(missing_instances.items(), key=lambda x: x[1], reverse=True)
    
    return sorted_missing_instances

# Define your folders
instances_folder = "../DeletefreeSAS"
output_folder = "../out/hmax"

missing_instances = find_missing_instances(instances_folder, output_folder)

# Print results
for name, size in missing_instances:
    print(f"Missing: {name}.sas, Size: {size} bytes")

