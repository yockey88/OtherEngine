from other.pipeline import env

from typing import override 
  
from other.native import engine

if __name__ == "__main__":
  print("> running yockcraft build pipeline")
  env.init("yockcraft/config/toolchain.cfg")