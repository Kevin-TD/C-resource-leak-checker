import sys
import os

# Alias for whole_test_run.py for pass test
# Used only for convenience

# checks if cwd is build
if os.path.split(os.getcwd())[1] != "build":
    print(f"ERROR: not in build dir; cwd is {os.getcwd()}")
    sys.exit(1)

args = " ".join(sys.argv[1:])
exit_status = os.system(
    f"make ; python3 ../whole_test_run.py PassTestRunner Passes --no-make {args}")

if exit_status != 0:
    sys.exit(1)
