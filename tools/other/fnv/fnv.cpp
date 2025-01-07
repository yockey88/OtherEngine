#include "core/defines.hpp"
#include "core/formatters.hpp"

using namespace other;

void Usage() {
  println("FNV Hash Calculator");
  println("Usage: fnv <string>");
  println("   > Example: fnv hello");
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Usage();
    return 1;
  }

  std::string arg = argv[1];
  uint64_t fnv = FNV(arg);
  println("FNV({}) = {}", arg, fnv);

  return 0;
}
