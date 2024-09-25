/**
 * \file Native/specialized_tests/host_tests.cpp
 **/
#include <gtest/gtest.h>

#include <filesystem>
#include <print>

#include "core/dotest.hpp"

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"
#include "hosting/assembly.hpp"
#include "hosting/host.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/method.hpp"
#include "hosting/native_object.hpp"
#include "reflection/object_proxy.hpp"

using namespace dotother::literals;
using namespace std::string_view_literals;

using dotother::MessageLevel;

class HostTests : public DoTest {
  public:
  protected:
    dotother::HostConfig config = {
      .host_config_path = DO_STR("./DotOther/Managed/DotOther.Managed.runtimeconfig.json") ,
      .managed_asm_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.dll") ,
      .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed") ,
      .entry_point = DO_STR("EntryPoint") ,
      
      .log_level = dotother::MessageLevel::INFO,

      .exception_callback = [](const dotother::NString message) {
        dotother::util::print(DO_STR("C# Exception Caught : \n\t{}"sv), MessageLevel::DEBUG, message);
        FAIL();
      } ,
      .log_callback = [](const dotother::NString message, dotother::MessageLevel level) {
        bool is_verbose = dotother::util::GetUtils().IsVerbose();
        if (!is_verbose && level < dotother::MessageLevel::INFO) {
          return;
        }

        fmt::print("{} [{}]\n"sv, message, level);
      } ,      
      .invoke_native_method_hook = [](uint64_t object_handle, const dotother::NString method_name) {
        std::string mname = method_name;
        dotother::util::print(DO_STR("Invoking Native Method on object {:#08x}"sv) , MessageLevel::DEBUG , object_handle);
        dotother::util::print(DO_STR(" > Method Name: {}"sv) , MessageLevel::DEBUG, mname);

        dotother::InteropInterface::Instance().InvokeNativeFunction(object_handle, mname);
      },

      .internal_logging_hook = [](const std::string_view message, dotother::MessageLevel level, bool verbose) {
        bool is_verbose = dotother::util::GetUtils().IsVerbose();
        if (!is_verbose && level < dotother::MessageLevel::INFO) {
          return;
        }

        fmt::print("{} [{}]\n"sv, message, level);
      }
    };

    static dotother::owner<dotother::NObject> native_object;

    virtual void SetUp() {
      host = dotother::Host::Instance(config);
      ASSERT_NO_THROW(host->LoadHost());
      ASSERT_NO_THROW(host->CallEntryPoint());
      
      native_object = dotother::new_owner<dotother::NObject>(0xdeadbeef);
      dotother::util::print(DO_STR("Native Object Handle: {:#08x}"sv) , MessageLevel::DEBUG, native_object->handle);
    }

    virtual void TearDown() {
      ASSERT_NO_FATAL_FAILURE(host->UnloadHost());

      dotother::InteropInterface::Unbind();
      dotother::Host::Destroy();
    }

  public:
    static void* GetNativeObject() {
      dotother::util::print(DO_STR("Getting Native Object Handle: {:#08x} (address : {:p})"sv), MessageLevel::DEBUG, native_object->handle , fmt::ptr(&native_object));
      return native_object.get();
    }

  protected:
    dotother::Host* host = nullptr;
};

dotother::owner<dotother::NObject> HostTests::native_object = nullptr;

TEST_F(HostTests, load_asm_and_call_functions) {
  /// set values through proxy
  native_object->proxy->handle(0xdeadbeef);
  dotother::util::print(DO_STR("Native Object Handle: {:#08x}"sv), MessageLevel::DEBUG, native_object->handle);
  ASSERT_EQ(native_object->handle , 0xdeadbeef);
  
  /// invoke functions by name (so client scripts can call them)
  native_object->proxy->InvokeMethod("Test");

  /// CallEntryPoint throws if interop is not bound before entry point is called, so we can assume it is bound here
  ///     and this must return to true
  ASSERT_TRUE(host->GetInteropInterface().BoundToAsm());

  dotother::AssemblyContext asm_ctx;
  ASSERT_NO_THROW(asm_ctx = host->CreateAsmContext("Module1"));
  
  ASSERT_NE(asm_ctx.context_id, -1);
  dotother::util::print(DO_STR("Assembly Context ID : {}"sv) , MessageLevel::DEBUG, asm_ctx.context_id);

  std::filesystem::path mod1_path = "./bin/Debug/DotOther.Tests/net8.0/Mod1.dll";
  
  dotother::ref<dotother::Assembly> assembly = nullptr;
  ASSERT_NO_THROW(assembly = asm_ctx.LoadAssembly(mod1_path.string()));
  ASSERT_NE(assembly , nullptr);
  ASSERT_NE(assembly->GetId(), -1);
  
  assembly->SetInternalCall("DotOther.Tests.Mod1", "GetNativeObject", (void*)&HostTests::GetNativeObject);
  ASSERT_NO_THROW(assembly->UploadInternalCalls());
  
  auto& type = assembly->GetType("DotOther.Tests.Mod1");
  ASSERT_NE(type.handle, -1);

  dotother::util::print(DO_STR("Creating Instance Type: {}"sv) , MessageLevel::DEBUG, type.FullName());
  dotother::HostedObject obj;
  ASSERT_NO_FATAL_FAILURE(obj = type.NewInstance());

  ASSERT_NO_FATAL_FAILURE(obj.Invoke("Test"));
  ASSERT_NO_FATAL_FAILURE(obj.Invoke("Test", 22));

  dotother::util::print(DO_STR("Invoking Internal Call"sv) , MessageLevel::DEBUG);
  ASSERT_NO_FATAL_FAILURE(obj.Invoke("TestInternalCall"));

  float number = 0.f;
  float expected = 42.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetField("number", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetField<float>("number"sv));
  ASSERT_EQ(number , expected);
  dotother::util::print(DO_STR("Field Value: {}"sv) , MessageLevel::DEBUG, number);

  expected = 69.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetProperty("MyNum", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetProperty<float>("MyNum"sv));
  ASSERT_EQ(number , expected);
  dotother::util::print(DO_STR("Property Value: {}"sv) , MessageLevel::DEBUG, number);

  ASSERT_NO_THROW(host->UnloadAssemblyContext(asm_ctx));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
