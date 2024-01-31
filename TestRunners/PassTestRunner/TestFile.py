# TODO:  make abstract version to be extended for other test runners

import sys

sys.path.insert(0, '..')
from TestRunners.Utils import *

class TestFile:
    """ represents a C file from the test directory. holds information on whether its IR 
    will be generated or if that file will be ran. 
    """

    INIT_TEST_CONDITIONS = {
        "IR_WILL_GENERATE": True, 
        "TEST_WILL_RUN": True, 
    }
    
    def __init__(self, file_path: str):
        """
        Args:
            file_path (str): must be exact path to some c file that starts with 
            "../test/[test name]/ 
        """
        self.__file_path: str = file_path

        try:
            self.__file_name: str = file_path[find_nth(file_path, "/", 3) + 1:]
        except Exception:
            raise FileNotFoundError(f"File path {file_path} has unexpected format; must begin with '../test/[test name]/'")
    
        self.__ir_will_generate: bool = self.INIT_TEST_CONDITIONS["IR_WILL_GENERATE"]
        self.__test_will_run: bool = self.INIT_TEST_CONDITIONS["TEST_WILL_RUN"]

    def toggle_ir_generation(self):
        self.__ir_will_generate = not self.__ir_will_generate
    
    def toggle_test_running(self):
        self.__test_will_run = not self.__test_will_run

    def ir_will_generate(self) -> bool:
        return self.__ir_will_generate

    def test_will_run(self) -> bool:
        return self.__test_will_run

    def get_file_path(self) -> str:
        return self.__file_path

    def get_file_name(self) -> str:
        """file name is file_path but with ../test/[test name]/ removed. used 
        when the user references a file name to do something with in the flags. 
        e.g., "python3 ../run_test.py simple_layer_test --no-build-ir-for test1.c"
        we'll be able to know the user is referencing ../test/simple_layer_test/test1.c. 
        works if the file is in a subdir as well. e.g., 
        "python 3 ../run_test.py simple_layer_test --no-build-ir-for layer/test1_again.c"                    
        """
        return self.__file_name