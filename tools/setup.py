from setuptools import setup, find_packages
import sys

if sys.version_info < (3, 12):
    sys.exit('Python 3.12 or later is required')

if __name__ == "__main__":
  setup(
          name='other',
          version='0.1',
          description='Other Engine',
          license='MIT',
          packages=find_packages(),
          zip_safe=False
        )