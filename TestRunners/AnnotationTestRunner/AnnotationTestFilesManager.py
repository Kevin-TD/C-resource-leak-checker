import sys

sys.path.insert(0, '..')
from TestRunners.TestFilesManager import TestFilesManager


class AnnotationTestFilesManager(TestFilesManager):
    """ holder for annotation test files
    """

    def __init__(self):
        super().__init__()
