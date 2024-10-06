/**
 * \file Native/specialized_tests/host_tests.cpp
 **/
#include <gtest/gtest.h>

#include <filesystem>
#include <print>

#include <glm/glm.hpp>

#include "core/dotest.hpp"

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"
#include "hosting/assembly.hpp"
#include "hosting/host.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/method.hpp"
#include "hosting/native_object.hpp"
#include "reflection/object_proxy.hpp"

using namespace dotother;
using namespace dotother::literals;

using namespace std::string_view_literals;

class HostTests : public DoTest {
  public:
  protected:
    HostConfig config = {
      .host_config_path = DO_STR("./DotOther/Managed/DotOther.Managed.runtimeconfig.json") ,
      .managed_asm_path = DO_STR("./bin/Debug/DotOther.Managed/net8.0/DotOther.Managed.dll") ,
      .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed") ,
      .entry_point = DO_STR("EntryPoint") ,

      .exception_callback = [](const NString message) {
        util::print(DO_STR("C# Exception Caught : \n\t{}"sv), MessageLevel::DEBUG, message);
        FAIL();
      } ,
      .log_callback = [](const NString message, MessageLevel level) {
        fmt::print("[MANAGED LOG HOOK] > {} [{}]\n"sv, message, level);
      } ,      
      .invoke_native_method_hook = [](uint64_t object_handle, const NString method_name) {
        std::string mname = method_name;
        util::print(DO_STR("Invoking Native Method on object {:#08x}"sv) , MessageLevel::DEBUG , object_handle);
        util::print(DO_STR(" > Method Name: {}"sv) , MessageLevel::DEBUG, mname);
        InteropInterface::Instance().InvokeNativeFunction(object_handle, mname);
      },

      .internal_logging_hook = [](const std::string_view message, MessageLevel level) {
        fmt::print("[INTERNAL LOGGING HOOK] > {} [{}]\n"sv, message, level);
      }
    };

    static owner<NObject> native_object;
    static inline glm::vec3 vec = glm::vec3(1.f, 2.f, 3.f); // , 4.f);

    virtual void SetUp() {
      host = Host::Instance(config);
      ASSERT_NO_THROW(host->LoadHost());
      ASSERT_NO_THROW(host->CallEntryPoint());
      
      native_object = new_owner<NObject>(0xdeadbeef);
      util::print(DO_STR("Native Object Handle: {:#08x}"sv) , MessageLevel::DEBUG, native_object->handle);
    }

    virtual void TearDown() {
      ASSERT_NO_FATAL_FAILURE(host->UnloadHost());

      InteropInterface::Unbind();
      Host::Destroy();
    }

  public:
    static void* GetVec3() {
      util::print(DO_STR("Getting Native Object Handle: {:#08x} (address : {:p})"sv), MessageLevel::DEBUG, native_object->handle , fmt::ptr(&native_object));
      return &vec;
    }

  protected:
    Host* host = nullptr;
};

owner<NObject> HostTests::native_object = nullptr;

TEST_F(HostTests, load_asm_and_call_functions) {
  /// set values through proxy
  native_object->proxy->handle(0xdeadbeef);
  util::print(DO_STR("Native Object Handle: {:#08x}"sv), MessageLevel::DEBUG, native_object->handle);
  ASSERT_EQ(native_object->handle , 0xdeadbeef);

  /// CallEntryPoint throws if interop is not bound before entry point is called, so we can assume it is bound here
  ///     and this must return to true
  ASSERT_TRUE(host->GetInteropInterface().BoundToAsm());

  AssemblyContext asm_ctx;
  ASSERT_NO_THROW(asm_ctx = host->CreateAsmContext("Module1"));
  
  ASSERT_NE(asm_ctx.context_id, -1);
  util::print(DO_STR("Assembly Context ID : {}"sv) , MessageLevel::DEBUG, asm_ctx.context_id);

  std::filesystem::path mod1_path = "./bin/Debug/DotOther.Tests/net8.0/Mod1.dll";
  
  ref<Assembly> assembly = nullptr;
  ASSERT_NO_THROW(assembly = asm_ctx.LoadAssembly(mod1_path.string()));
  ASSERT_NE(assembly , nullptr);
  ASSERT_NE(assembly->GetId(), -1);

  assembly->SetInternalCall("DotOther.Tests.Mod1" , "GetVec3" , (void*)&HostTests::GetVec3);
  ASSERT_NO_FATAL_FAILURE(assembly->UploadInternalCalls());
  
  Type& type = assembly->GetType("DotOther.Tests.Mod1");
  ASSERT_NE(type.handle, -1);

  util::print(DO_STR("Creating Instance Type: {}"sv) , MessageLevel::DEBUG, type.FullName());
  HostedObject obj;
  ASSERT_NO_FATAL_FAILURE(obj = type.NewInstance());

  ASSERT_NO_FATAL_FAILURE(obj.Invoke<void>("Test"));
  ASSERT_NO_FATAL_FAILURE(obj.Invoke<void>("Test", 22));
  ASSERT_NO_FATAL_FAILURE(obj.SetProperty("MyInt", 22));

  util::print(DO_STR("Invoking Internal Call"sv) , MessageLevel::DEBUG);
  ASSERT_NO_FATAL_FAILURE(obj.Invoke<void>("TestInternalCall"));

  float number = 0.f;
  float expected = 42.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetField("number", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetField<float>("number"sv));
  ASSERT_EQ(number , expected);
  util::print(DO_STR("Field Value: {}"sv) , MessageLevel::DEBUG, number);

  expected = 69.f;
  ASSERT_NO_FATAL_FAILURE(obj.SetProperty("MyNum", expected));
  ASSERT_NO_FATAL_FAILURE(number = obj.GetProperty<float>("MyNum"sv));
  ASSERT_EQ(number , expected);
  util::print(DO_STR("Property Value: {}"sv) , MessageLevel::DEBUG, number);

  ASSERT_NO_THROW(host->UnloadAssemblyContext(asm_ctx));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
