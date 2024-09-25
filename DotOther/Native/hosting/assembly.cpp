/**
 * \file hosting/assembly.cpp
 **/
#include "hosting/assembly.hpp"

#include <cstdint>
#include <filesystem>
#include <string_view>

#include "core/utilities.hpp"
#include "hosting/host.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/native_string.hpp"
#include "hosting/type_cache.hpp"

namespace dotother {

  int32_t Assembly::GetId() const {
    return asm_id;
  }

  AssemblyLoadStatus Assembly::LoadStatus() const {
    return load_status;
  }

  std::string_view Assembly::Name() const {
    return name;
  }

  Type& Assembly::GetType(const std::string_view name) const {
    static Type null_type(-1);
    Type* type = TypeCache::Instance().GetType(name);
    return type != nullptr ? 
      *type : null_type;
  }

  std::string Assembly::GetAsmQualifiedName(const std::string_view klass, const std::string_view method_name) const {
    return (std::string {klass} + "+" + 
            std::string{method_name} + ", " + name);
  }

  void Assembly::SetInternalCall(const std::string_view klass, const std::string_view method_name , void* fn) {
    using namespace std::string_view_literals;
    if (fn == nullptr) {
      util::GetUtils().log_sink(fmt::format("Attempting to register an internall call ({}.{}) using a null pointer!"sv, klass, method_name),
                                MessageLevel::ERR, true);
      return;
    }

    std::string asm_name{ klass };
    asm_name += "+";
    asm_name += method_name;
    asm_name += ", ";
    asm_name += name;

    const dostring& internal_name = internal_call_names.emplace_back(util::CharToWide(asm_name));
    internal_calls.push_back({
      .name = internal_name.c_str() ,
      .native_function = fn
    });
  }

  void Assembly::UploadInternalCalls() {
    if (!Interop().BoundToAsm()) {
      util::print(DO_STR("Interop is not bound to assembly!"), MessageLevel::CRITICAL);
      throw std::runtime_error("Interop is not bound to assembly!");
    }

    if (internal_call_names.empty()) {
      return;
    }
    Interop().set_internal_calls(internal_calls.data() , static_cast<int32_t>(internal_calls.size()));
  }

  void Assembly::AddCall(const std::string& name , void* fn) {

  }

  ref<Assembly> AssemblyContext::LoadAssembly(const std::string_view path) {
    NString filepath = NString::New(path);
    std::filesystem::path p = (std::string)filepath;
    if (!std::filesystem::exists(p)) {
      NString::Free(filepath);
      util::print(DO_STR("Assembly does not exist: {}"), MessageLevel::ERR , path);
      return nullptr;
    }

    // Host* host = Host::Instance();

    size_t idx = assemblies.size();
    auto& assembly = assemblies.emplace_back(new_ref<Assembly>());
    assembly->asm_id = Interop().load_assembly(context_id, filepath);

    util::print(DO_STR("Assembly ID: {}"), MessageLevel::INFO , assembly->asm_id);
    if (assembly->asm_id == -1) {
      NString::Free(filepath);
      assemblies.pop_back();
      util::print(DO_STR("Failed to load assembly file: {}"), MessageLevel::ERR , path);
      return nullptr;
    }

    assembly->load_status = Interop().get_last_load_status();
    if (assembly->load_status == AssemblyLoadStatus::FILE_LOAD_FAILED) {
      NString::Free(filepath);
      assemblies.pop_back();
      util::print(DO_STR("Failed to load assembly file: {}"), MessageLevel::ERR, path);
      return nullptr;
    }

    if (assembly->load_status == AssemblyLoadStatus::SUCCESS) {
      auto asm_name = Interop().get_assembly_name(assembly->asm_id);
      assembly->name = asm_name;
      NString::Free(asm_name);
      util::print(DO_STR("Assembly loaded successfully! {}"), MessageLevel::INFO, assembly->name);

      int32_t type_counter = 0;
      Interop().get_asm_types(assembly->asm_id, nullptr, &type_counter);

      util::print(DO_STR("Loading [{}] types"), MessageLevel::INFO, type_counter);
      
      std::vector<int32_t> type_ids(type_counter);
      Interop().get_asm_types(assembly->asm_id, type_ids.data(), &type_counter);

      for (auto id : type_ids) {
        util::print(DO_STR("Loading type with ID: {}"), MessageLevel::INFO , id);

        Type type(id);
        auto t = assembly->types.emplace_back(TypeCache::Instance().CacheType(std::move(type)));
        util::print(DO_STR("  > Type loaded: {}"), MessageLevel::INFO, t->FullName());
      }
    }

    NString::Free(filepath);
    return assembly;
  }

  const std::vector<ref<Assembly>>& AssemblyContext::GetAssemblies() const {
    return assemblies;
  }

} // namespace dotother
