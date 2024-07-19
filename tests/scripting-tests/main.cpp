/**
 * \file tests/scripting-tests/main.cpp
 **/
#include <spdlog/fmt/fmt.h>

#include <sol/sol.hpp>

#include "core/defines.hpp"

using other::Path;

class C {
  public:
    enum class Number : uint32_t {
      ZERO = 0 ,
      ONE = 1 , 
      TWO = 2 , 
      THREE = 3
    };

    static bool CheckNum(Number num) {
      return num == Number::THREE;
    }
};

int main(int argc , char* argv[]) {
  /// setup
  sol::state lua;

  lua.open_libraries(sol::lib::io , sol::lib::string);

  lua["Number"] = lua.create_table_with(
    "Zero" , C::Number::ZERO , 
    "One" , C::Number::ONE , 
    "Two" , C::Number::TWO , 
    "Three" , C::Number::THREE
  );

  lua.set_function("print" , [](const std::string& str) { std::cout << str << "\n"; });

  lua.new_usertype<C>(
    "Class" ,
    "CheckNum" , C::CheckNum 
  );

  lua.script(R"(
    if Class.CheckNum(Number.Three) then
      print("Hell ya!");
    end
  )");

  return 0;
}
