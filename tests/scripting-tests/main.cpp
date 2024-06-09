/**
 * \file tests/scripting-tests/main.cpp
 **/
#include "scripting/script_engine.hpp"
#include <algorithm>
#include <plugin/plugin_loader.hpp>
#include <scripting/language_module.hpp>

int main(int argc , char* argv[]) {
  std::string str = "string";
  std::cout << str << " " << other::FNV(str) << std::endl;
  std::transform(str.begin() , str.end() , str.begin() , ::toupper);
  std::cout << str << " " << other::FNV(str) << std::endl;

  return 0;
}
