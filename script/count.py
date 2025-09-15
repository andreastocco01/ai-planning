import os

# Set your target directory here
#directory = '../out/backprop_min_ucs_500000_a'
directory = 'test/'

# Search term
search_term = r"Solution does not exist!"
search_term2 = r"############### Solution ###############"

count = 0

# Loop through files in the directory
for filename in os.listdir(directory):
    file_path = os.path.join(directory, filename)
    
    # Only check regular files (skip directories)
    if os.path.isfile(file_path):
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                contents = f.read()
                f.seek(0)
                lines = f.readlines()
                if "Cost" not in lines[-1] and "Timelimit reached" not in lines[-1] and "Solution does not exist!" not in lines[-1]:
                    print(lines[-1], file_path)
                if search_term in contents or search_term2 in contents:
                    count += 1
                else:
                    pass
                    #print(filename)
        except Exception as e:
            print(f"Could not read file {filename}: {e}")
print(str(count) + "/" + str(len(os.listdir(directory))))

