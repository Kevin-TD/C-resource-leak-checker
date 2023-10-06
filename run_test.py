#! python3 ../run_test.py <folder name in /test> 
# must be in build dir
# e.g., python3 ../run_test.py nesting_test 

# takes a test folder in the /test directory and runs every .c 
# file in it, including those in subdirectories 

import os 
import sys 

# checks if cwd is build
if os.path.split(os.getcwd())[1] != "build":
    print(f"WARNING: not in build dir; cwd is {os.getcwd()}")
    sys.exit(1)


test_folder_name = sys.argv[1]

def get_all_c_files(folder_path: str, collected_files = []):
    with os.scandir(folder_path) as entries:
        for entry in entries:
            if entry.is_file() and entry.name.endswith(".c"):
                collected_files.append(f"{folder_path}/{entry.name}")

            elif entry.is_dir():
                get_all_c_files(f"{folder_path}/{entry.name}", collected_files)
    
    return collected_files


c_files = get_all_c_files(f"../test/{test_folder_name}")

print("Making analysis")
os.system("make")

for c_file in c_files:
    c_file_as_ll = c_file.replace(".c", ".ll")
    command = f"clang -emit-llvm -g -S -fno-discard-value-names -Xclang -disable-O0-optnone -c {c_file} -o {c_file_as_ll} ; opt -load CodeAnalyzer.so -CodeAnalyzer {c_file_as_ll}"
    print(f"Running command: {command}")
    os.system(command)

