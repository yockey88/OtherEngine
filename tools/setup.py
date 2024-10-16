import sys

from glob import glob
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension

if sys.version_info < (3, 12):
    sys.exit('Python 3.12 or later is required')

if __name__ == "__main__":
  setup(
          name='other',
          version='0.1',
          description='Other Engine',
          license='MIT',
          packages=find_packages(exclude=['*/bin/*']),
          include_package_data=True,
          zip_safe=False
        )