/**
 * \file dotother_tests/main.cpp
 **/
#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "nethost.h"

#include <Windows.h>
#include <ShlObj_core.h>

#include <iostream>
#include <filesystem>

#include "defines.hpp"
#include "native_string.hpp"
#include "host.hpp"

class TestClass {
public:

};

int main(int argc , char* argv[]) {
  std::filesystem::current_path("C:/Yock/code/OtherEngine");

  dotother::HostConfig config = {
    .host_config_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.runtimeconfig.json") ,
    .managed_asm_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.dll") ,
    .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed") ,
    .entry_point = DO_STR("EntryPoint") ,

    .exception_callback = [](const dotother::NString message) {
      std::wcout << message.Data() << std::endl;
    } ,
    .log_callback = [](const dotother::NString message, dotother::MessageLevel level) {
      std::wcout << message.Data() << DO_STR(" [") << (int32_t)level  << DO_STR("]") << std::endl;
    } ,
  };
  std::cout << "Loading host config\n";
  dotother::Host host(config);

  if (!host.LoadHost()) {
    std::cout << "Failed to load host" << std::endl;
    return 1;
  }
  host.CallEntryPoint();



  /// do other stuff here

  host.UnloadHost();
  return 0;
}
