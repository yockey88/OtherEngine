/**
 * \file assembly.hpp
 **/
#ifndef DOTOTHER_NATIVE_ASSEMBLY_HPP
#define DOTOTHER_NATIVE_ASSEMBLY_HPP

#include "defines.hpp"
#include <filesystem>
#include <string>
#include <string_view>

namespace dotother {

  class Host;

  class Assembly {
    public:
      int32_t GetId() const;
      AssemblyLoadStatus LoadStatus() const;
      std::string_view Name() const;

      void SetInternalCall(const std::string_view klass, const std::string_view call_name, void* fn);
      // void UploadInternalCalls();

      // Type& GetType(const std::string_view klass) const;
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

      // std::vector<Type*> types = {};

      friend class Host;
      friend class AssemblyContext;
  };

  class AssemblyContext {
    public:
      Assembly& LoadAssembly(const std::string_view path);
      const std::vector<Assembly> GetAssemblies() const;

      int32_t context_id = -1;

    private:
      std::vector<Assembly> assemblies{};

      Host* host = nullptr;
      friend class Host;
  };

} // namespace dotother

#endif // !DOTOTHER_NATIVE_ASSEMBLY_HPP