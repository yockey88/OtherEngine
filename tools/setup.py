import sys

if sys.version_info < (3, 12):
    sys.exit('Python 3.12 or later is required')
    
import subprocess
import os
from glob import glob
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext, ParallelCompile, naive_recompile

compilation_args = [];
if sys.platform == "win32":
    compilation_args = ["/std:c++latest", "/Zc:preprocessor", "/Zc:__cplusplus", "/O3"]
elif sys.platform == "linux":
    compilation_args = ["-std=c++23", "-O3"]
else:
    raise EnvironmentError("Unsupported platform detected")

ParallelCompile(default=16).install()

def get_source_files():
    source_files = []
    source_files.append("./other/native/othernative.cpp")
    source_files.extend(glob("../otherenv/src/*.cpp"))
    source_files.extend(glob("../otherenv/src/**/*.cpp"))

    if sys.platform == "win32":
        source_files.extend(glob("../otherenv/platform/windows/*.cpp"))
    elif sys.platform == "linux":
        source_files.extend(glob("../otherenv/platform/linux/*.cpp"))
    else:
        raise EnvironmentError("Unsupported platform detected")
    
    return source_files    

def get_include_dirs():
    include_dirs = []
    include_dirs.append("../otherenv/src")
    include_dirs.append("../externals/pybind11")
    include_dirs.append("../externals/choc")
    include_dirs.append("../externals/refl-cpp")
    include_dirs.append("../externals/glm")
    include_dirs.append("../externals/json/include")
    include_dirs.append("../externals/tomlplusplus")
    include_dirs.append("../externals/magic_enum")
    include_dirs.append("../externals/spdlog/include")
    include_dirs.append("../externals/sol2")
    include_dirs.append("../externals/sdl")
    include_dirs.append("../externals/imgui")
    include_dirs.append("../externals/glad/include")
    
    if sys.platform == "win32":
        include_dirs.append("../otherenv/platform/windows")
    elif sys.platform == "linux":
        include_dirs.append("../otherenv/platform/linux")
    else:
        raise EnvironmentError("Unsupported platform detected")
    
    return include_dirs
  
def get_libraries():
    libraries = []
    libraries.append("../bin/Debug/spdlog/spdlog")
    libraries.append("../bin/Debug/sol2/sol2")
    libraries.append("../externals/sdl/lib/Debug/SDL3")
    libraries.append("../bin/Debug/imgui/imgui")
    libraries.append("../bin/Debug/glad/glad")
    return libraries

## build othernative here, we have to duplicate the project building code to avoid using 
##  code from other toolchain in the other toolchain build process
ext_modules = [
    Pybind11Extension(
        "othernative",
        sources=get_source_files(),
        include_dirs=get_include_dirs(),
        libraries=get_libraries(),
        extra_compile_args=compilation_args,
    ),
]

packages = [];
packages.extend(find_packages())

if __name__ == "__main__":
  setup(
          name='other',
          version='0.1',
          description='Other Engine',
          license='MIT',
          packages=find_packages(),
          ext_modules = ext_modules,
          cmdclass={'build_ext': build_ext},
          include_package_data=True,
          zip_safe=False
        )