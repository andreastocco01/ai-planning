import os
import re

def find_incomplete_outputs(instances_folder, output_folder):
    # Get all instance file names (without extensions)
    instance_files = {f.rsplit(".", 1)[0]: os.path.getsize(os.path.join(instances_folder, f))
                      for f in os.listdir(instances_folder) if f.endswith(".sas")}
    
    # Extract instance names from output files
    output_files = {}
    pattern = re.compile(r"^(.*)_\w+_\d+\.out$")
    
    for f in os.listdir(output_folder):
        match = pattern.match(f)
        if match:
            output_files[match.group(1)] = os.path.join(output_folder, f)
    
    # Find instances with an output file but missing the last line containing "Cost"
    incomplete_instances = []
    for instance, output_path in output_files.items():
        try:
            with open(output_path, 'r', encoding='utf-8') as file:
                lines = file.readlines()
                if not lines or "Cost" not in lines[-1]:
                    incomplete_instances.append((instance, instance_files.get(instance, 0)))
        except Exception as e:
            print(f"Error reading {output_path}: {e}")
    
    # Sort instances by size in descending order
    incomplete_instances.sort(key=lambda x: x[1], reverse=True)
    
    return incomplete_instances

# Define your folders
instances_folder = "../DeletefreeSAS"
output_folder = "../out/hmax"

incomplete_instances = find_incomplete_outputs(instances_folder, output_folder)

# Print results
for name, size in incomplete_instances:
    print(f"Incomplete: {name}.sas, Size: {size} bytes")
