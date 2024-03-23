#include "other_engine.hpp"

#include <iostream>

int main(int argc , char* argv[]) {
  try {
    other::ExitCode exit = other::OE::Main(argc , argv);
    if (exit != other::ExitCode::SUCCESS) {
      return 1;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
