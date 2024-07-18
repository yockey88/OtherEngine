/**
 * \file tests/scripting-tests/main.cpp
 **/
#include <spdlog/fmt/fmt.h>

#include <sol/sol.hpp>

#include "core/defines.hpp"
#include "core/filesystem.hpp"

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

#if 0 // enum and statuc function tests
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
#else
  // std::vector<Path> premake_paths;
  // Path premake_dir = "./OtherEngine-ScriptCore/lua/premake";
  // Path vstudio_dir = "./OtherEngine-ScriptCore/lua/vstudio";
  // Path main_file;
  // for (auto entry : std::filesystem::recursive_directory_iterator(premake_dir)) {
  //   if (!entry.is_regular_file()) {
  //     continue;
  //   }

  //   if (entry.path().filename().string() == "_premake_main.lua") {
  //     main_file = entry.path();
  //     continue;
  //   } else if (entry.path().extension().string() != ".lua") {
  //     continue;
  //   } 

  //   lua.load_file(entry.path().string());
  // }
  // for (auto entry : std::filesystem::recursive_directory_iterator(vstudio_dir)) {
  //   lua.load_file(entry.path().string());
  // }

  // lua["getEmbeddedResources"] = [](lua_State* L) {
  //   const char* filename = luaL_checkstring(L , 1);
  //   const buildin_mapping* chunk = premake_find_embedded_script(filename);
  //   if (chunk == nullptr) {
  //     return 0;
  //   }

  //   lua_pushlstring(L , (const char*)chunk->bytecode , chunk->length);
  //   return 1;
  // };

  const char* args[] = {
    "premake" , "vs2022"
  };

  // Path premake_path = other::Filesystem::GetEngineCoreDir() / "premake" / "premake5.exe";
  // std::string cmd = other::fmtstr("{} vs2022" , premake_path.string());
  // std::replace(cmd.begin() , cmd.begin() , '/' , '\\');

  // std::cout << cmd << std::endl;

  // system(cmd.c_str());

#endif 

  return 0;
}
