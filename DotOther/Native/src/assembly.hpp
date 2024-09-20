/**
 * \file assembly.hpp
 **/
#ifndef DOTOTHER_NATIVE_ASSEMBLY_HPP
#define DOTOTHER_NATIVE_ASSEMBLY_HPP

#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest_prod.h> /// for GTEST_FRIEND_TEST

#include "defines.hpp"
#include "type.hpp"

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

      // void SetInternalCall(const std::string_view klass, const std::string_view call_name, void* fn);
      // void UploadInternalCalls();

      Type& GetType(const std::string_view klass) const;
      // const std::vector<Type*>& GetTypes() const;

    private:
      Host* host = nullptr;
      int32_t asm_id = -1;
      AssemblyLoadStatus load_status = AssemblyLoadStatus::UNKNOWN_ERROR;

      std::string name;

#ifdef DOTOTHER_WIDE_CHARS
      std::vector<std::wstring> internal_call_names = {};
#else
      std::vector<std::string> internal_call_names = {};
#endif // !DOTOTHER_WIDE_CHARS

      std::vector<InternalCall> internal_calls = {};

      std::vector<Type*> types = {};

      friend class Host;
      friend class AssemblyContext;
      
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