/**
 * \file hosting/assembly.hpp
 **/
#ifndef DOTOTHER_NATIVE_ASSEMBLY_HPP
#define DOTOTHER_NATIVE_ASSEMBLY_HPP

#include <string>
#include <string_view>
#include <vector>
#include <functional>

#include <gtest/gtest_prod.h> /// for GTEST_FRIEND_TEST

#include "core/defines.hpp"
#include "hosting/type.hpp"
#include "hosting/interop_interface.hpp"
#include "reflection/echo_type.hpp"

class DoTest;
class HostTests;

namespace dotother {

  class Host;

  class Assembly {
    public:
      Assembly(Host* host)
        : host(host) {}

      int32_t GetId() const;
      AssemblyLoadStatus LoadStatus() const;
      std::string_view Name() const;

      void SetInternalCall(const std::string_view klass, const std::string_view name , void* fn);
      void UploadInternalCalls();

      Type& GetType(const std::string_view klass) const;
      // const std::vector<Type*>& GetTypes() const;

    private:
      Host* host = nullptr;
      int32_t asm_id = -1;
      AssemblyLoadStatus load_status = AssemblyLoadStatus::UNKNOWN_ERROR;

      std::string name;

      std::vector<dostring> internal_call_names = {};

      std::vector<InternalCall> internal_calls = {};

      std::vector<Type*> types = {};

      std::string GetAsmQualifiedName(const std::string_view klass, const std::string_view method_name) const;

      void AddCall(const std::string& name , void* fn);

      friend class Host;
      friend class AssemblyContext;

      friend class DoTest;
      FRIEND_TEST(HostTests, load_asm_and_call_functions);
  };

  class AssemblyContext {
    public:
      AssemblyContext(Host* host)
        : host(host) {}

      ref<Assembly> LoadAssembly(const std::string_view path);
      const std::vector<ref<Assembly>>& GetAssemblies() const;

      int32_t context_id = -1;

    private:
      std::vector<ref<Assembly>> assemblies{};

      Host* host = nullptr;
      friend class Host;
  };

} // namespace dotother

#endif // !DOTOTHER_NATIVE_ASSEMBLY_HPP