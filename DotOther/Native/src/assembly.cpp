/**
 * \file assembly.cpp
 **/
#include "assembly.hpp"

#include <cstdint>

#include "interop_interface.hpp"
#include "native_string.hpp"

namespace dotother {

  Assembly& AssemblyContext::LoadAssembly(const std::string_view path) {
    auto filepath = NString::New(path);

    size_t idx = assemblies.size();
    auto& assembly = assemblies.emplace_back();
    assembly.host = host;
    assembly.asm_id = interop::load_managed_assembly(context_id, filepath);
    assembly.load_status = interop::get_last_load_status();

    if (assembly.load_status == AssemblyLoadStatus::SUCCESS) {
      auto asm_name = interop::get_assembly_name(assembly.asm_id);
      assembly.name = asm_name;
      NString::Free(asm_name);

      int32_t type_counter = 0;
      interop::get_asm_types(assembly.asm_id, nullptr, &type_counter);
      std::vector<int32_t> type_ids(type_counter);
      interop::get_asm_types(assembly.asm_id, type_ids.data(), &type_counter);

      for (auto id : type_ids) {
        // Type type;
        // type.id = id;
        // assembly.types.push_back(TypeCache::Get().CacheType(std::move(type)));
      }
    }
    NString::Free(filepath);
    return assembly;
  }

  const std::vector<Assembly> AssemblyContext::GetAssemblies() const {
    return assemblies;
  }

} // namespace dotother