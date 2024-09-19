/**
 * \file Tests/Managed/host_tests.cpp
 **/
#include "assembly.hpp"
#include "dotest.hpp"

#include <filesystem>
#include <iostream>

#include <gtest/gtest.h>

#include "host.hpp"
#include "defines.hpp"

using namespace dotother::literals;
using namespace std::string_view_literals;

class HostTests : public DoTest {
  public:
  protected:
    dotother::HostConfig config = {
      .host_config_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.runtimeconfig.json") ,
      .managed_asm_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.dll") ,
      .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed") ,
      .entry_point = DO_STR("EntryPoint") ,

      .exception_callback = [](const dotother::NString message) {
        dotother::util::print(DO_STR("Exception: {}"sv), message);
        FAIL();
      } ,
      .log_callback = [](const dotother::NString message, dotother::MessageLevel level) {
        dotother::util::print(DO_STR("{} [{}]"sv), message, level);
      } ,
    };

    void TearDown() override {
      if (host != nullptr) {
        host->UnloadHost();
      }
    }

    Scope<dotother::Host> host = nullptr;

    bool LoadHost() {
      if (host != nullptr) {
        host->UnloadHost();
      }

      host = std::make_unique<dotother::Host>(config);
      
      bool res = host->LoadHost();
      if (res) {
        host->CallEntryPoint();
      }
      return res;
    }
};

TEST_F(HostTests, load_fail) {
  auto pre_bad_path = config.host_config_path;
  config.host_config_path = DO_STR("./bad_path");
  {
    dotother::Host host(config);
    EXPECT_FALSE(host.LoadHost());
  }
  config.host_config_path = pre_bad_path;
  
  config.managed_asm_path = DO_STR("./bad_path");
  {
    dotother::Host host(config);
    EXPECT_FALSE(host.LoadHost());
  }
}

// static void PrintStringS(const dotother::NString message) {
//   dotother::util::print(DO_STR("Exception: {}"sv), message);
//   FAIL();
// }

TEST_F(HostTests, load_asm_and_call_functions) {
  ASSERT_TRUE(LoadHost());
  
  dotother::AssemblyContext asm_ctx;
  ASSERT_NO_FATAL_FAILURE(asm_ctx = host->CreateAsmContext("Tests.load_assembly"));
  std::cout << "Assembly ID : " << asm_ctx.context_id << std::endl;
  ASSERT_NE(asm_ctx.context_id, -1);

  std::filesystem::path mod1_path = "./bin/Debug/DotOther.Tests/net8.0/Mod1.dll";
  auto& assembly = asm_ctx.LoadAssembly(mod1_path.string());

  // auto& type = assembly.GetType("DotOther.Tests.Mod1");

  ASSERT_NO_FATAL_FAILURE(host->UnloadAssemblyContext(asm_ctx));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}