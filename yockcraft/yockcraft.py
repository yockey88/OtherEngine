from other.pipeline import env

from typing import override 
  

if __name__ == "__main__":
  print("> running yockcraft build pipeline")
  env.init("yockcraft/config/toolchain.cfg")