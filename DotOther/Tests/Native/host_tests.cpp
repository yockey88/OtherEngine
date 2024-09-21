/**
 * \file Tests/Native/host_tests.cpp
 **/
#include <gtest/gtest.h>

#include <filesystem>
#include <print>

#include "dotest.hpp"
#include "core/defines.hpp"
#include "core/utilities.hpp"
#include "hosting/assembly.hpp"
#include "hosting/host.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/method.hpp"
#include "hosting/native_object.hpp"

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
      
      .log_level = dotother::MessageLevel::INFO,

      .exception_callback = [](const dotother::NString message) {
        dotother::util::print(DO_STR("Exception: {}"sv), message);
        FAIL();
      } ,
      .log_callback = [](const dotother::NString message, dotother::MessageLevel level) {
        // bool is_verbose = dotother::util::GetUtils().IsVerbose();
        // if (!is_verbose && level < dotother::MessageLevel::INFO) {
        //   return;
        // }

        dotother::util::print(DO_STR("{} [{}]"sv), message, level);
      } ,

      .internal_logging_hook = [](const std::string_view message, dotother::MessageLevel level, bool verbose) {
        // bool is_verbose = dotother::util::GetUtils().IsVerbose();
        // if (!is_verbose && level < dotother::MessageLevel::INFO) {
        //   return;
        // }

        fmt::print("{} [{}]\n", message, level);
      }
    };

    void TearDown() override {
      if (host != nullptr) {
        host->UnloadHost();
      }
    }

    dotother::owner<dotother::Host> host = nullptr;

    static dotother::NObject native_object;

  public:
    static dotother::NObject& GetNativeObject() {
      return native_object;
    }
};

dotother::NObject HostTests::native_object = dotother::NObject();

TEST_F(HostTests, load_asm_and_call_functions) {
  host = dotother::new_owner<dotother::Host>(config);
  ASSERT_NO_THROW(host->LoadHost());
  ASSERT_NO_THROW(host->CallEntryPoint());

  /// CallEntryPoint throws if interop is not bound before entry point is called, so we can assume it is bound here
  ///     and this must return to true
  ASSERT_TRUE(host->GetInteropInterface().BoundToAsm());

  dotother::AssemblyContext asm_ctx(nullptr);
  ASSERT_NO_THROW(asm_ctx = host->CreateAsmContext("Module1"));
  
  ASSERT_NE(asm_ctx.context_id, -1);
  dotother::util::print(DO_STR("Assembly Context ID : {}"sv) , asm_ctx.context_id);

  std::filesystem::path mod1_path = "./bin/Debug/DotOther.Tests/net8.0/Mod1.dll";
  
  dotother::ref<dotother::Assembly> assembly = nullptr;
  ASSERT_NO_THROW(assembly = asm_ctx.LoadAssembly(mod1_path.string()));
  ASSERT_NE(assembly , nullptr);
  ASSERT_NE(assembly->GetId(), -1);
  
  assembly->SetInternalCall("DotOther.Tests.Mod1", "CreateObject", (void*)&HostTests::GetNativeObject);
  ASSERT_NO_THROW(assembly->UploadInternalCalls());

  auto& type = assembly->GetType("DotOther.Tests.Mod1");
  ASSERT_NE(type.handle, -1);

  dotother::util::print(DO_STR("Creating Instance Type: {}"sv), type.FullName());
  dotother::HostedObject obj;
  ASSERT_NO_FATAL_FAILURE(obj = type.NewInstance());

  ASSERT_NO_FATAL_FAILURE(obj.Invoke("Test"));
  ASSERT_NO_FATAL_FAILURE(obj.Invoke("Test", 22));

  float number = 0.f;
  float expected = 42.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetField("number", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetField<float>("number"sv));
  ASSERT_EQ(number , expected);
  dotother::util::print(DO_STR("Field Value: {}"sv), number);

  expected = 69.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetProperty("MyNum", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetProperty<float>("MyNum"sv));
  ASSERT_EQ(number , expected);
  dotother::util::print(DO_STR("Property Value: {}"sv), number);

  ASSERT_NO_THROW(host->UnloadAssemblyContext(asm_ctx));


}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}