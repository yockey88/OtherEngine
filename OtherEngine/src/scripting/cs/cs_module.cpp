/**
 * \file scripting/cs/cs_module.cpp
 */
#include "scripting/cs/cs_module.hpp"

#include <core/dotother_defines.hpp>
#include <core/stable_vector.hpp>
#include <hosting/garbage_collector.hpp>
#include <hosting/host.hpp>
#include <hosting/type_cache.hpp>

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/platform.hpp"

#include "application/app_state.hpp"

#include "scripting/cs/cs_script.hpp"

using dotother::Assembly;
using dotother::AssemblyContext;
using dotother::ref;
using dotother::StableVector;

namespace other {

  static dotother::Host* host = nullptr;

  namespace {

    struct DotOtherAssemblyContexts {
      std::map<UUID, int32_t> assembly_ids;
      std::map<int32_t, AssemblyContext> contexts;
    };

    static DotOtherAssemblyContexts assembly_contexts;

    static void ManagedLoggingCallback(const dotother::NString message, dotother::MessageLevel level) {
      switch (level) {
        case dotother::MessageLevel::TRACE:
          OE_TRACE(" [DotOther.Managed] : {}", message);
          break;
        case dotother::MessageLevel::DEBUG:
          OE_DEBUG(" [DotOther.Managed] : {}", message);
          break;
        case dotother::MessageLevel::MESSAGE:
        case dotother::MessageLevel::INFO:
          OE_INFO(" [DotOther.Managed] : {}", message);
          break;
        case dotother::MessageLevel::WARNING:
          OE_WARN(" [DotOther.Managed] : {}", message);
          break;
        case dotother::MessageLevel::ERR:
          OE_ERROR(" [DotOther.Managed] : {}", message);
          break;
        case dotother::MessageLevel::CRITICAL:
          OE_CRITICAL(" [DotOther.Managed] : {}", message);
          break;
      }
    }

    static void NativeLoggingCallback(const std::string_view message, dotother::MessageLevel level) {
      switch (level) {
        case dotother::MessageLevel::MESSAGE:
        case dotother::MessageLevel::TRACE:
          OE_TRACE(" [DotOther.Native] : {}", message);
          break;
        case dotother::MessageLevel::DEBUG:
          OE_DEBUG(" [DotOther.Native] : {}", message);
          break;
        case dotother::MessageLevel::INFO:
          OE_INFO(" [DotOther.Native] : {}", message);
          break;
        case dotother::MessageLevel::WARNING:
          OE_WARN(" [DotOther.Native] : {}", message);
          break;
        case dotother::MessageLevel::ERR:
          OE_ERROR(" [DotOther.Native] : {}", message);
          break;
        case dotother::MessageLevel::CRITICAL:
          OE_CRITICAL(" [DotOther.Native] : {}", message);
          break;
      }
    }

  }  // anonymous namespace

  using namespace std::string_view_literals;
  using namespace dotother::literals;

  namespace {

    ValueType GetValueTypeFromHash(uint64_t hash) {
      switch (hash) {
        case FNV("System.Boolean"):
          return ValueType::BOOL;
        case FNV("System.Char"):
          return ValueType::CHAR;
        case FNV("System.Byte"):
          return ValueType::UINT8;
        case FNV("System.Int8"):
          return ValueType::INT8;
        case FNV("System.Int16"):
          return ValueType::INT16;
        case FNV("System.Int32"):
          return ValueType::INT32;
        case FNV("System.Int64"):
          return ValueType::INT64;
        case FNV("System.UInt8"):
          return ValueType::UINT8;
        case FNV("System.UInt16"):
          return ValueType::UINT16;
        case FNV("System.UInt32"):
          return ValueType::UINT32;
        case FNV("System.UInt64"):
          return ValueType::UINT64;
        case FNV("System.Single"):
          return ValueType::FLOAT;
        case FNV("System.Double"):
          return ValueType::DOUBLE;
        case FNV("System.String"):
          return ValueType::STRING;

        default:
          return ValueType::EMPTY_TYPE;
      }
    }

  }  // anonymous namespace

  bool CsModule::Initialize() {
    try {
      const Path engine_core_dir = Filesystem::GetEngineCoreDir();
      const Path dotother_dir = engine_core_dir / "DotOther";
      const Path bin_dir = engine_core_dir / "bin" / "Debug" / "DotOther.Managed" / "net8.0";
      const Path managed_asm_dir = dotother_dir / "Managed";

      const Path host_config_path = managed_asm_dir / "DotOther.Managed.runtimeconfig.json";
      const Path managed_asm_path = bin_dir / "DotOther.Managed.dll";

      const dotother::HostConfig config = {
        /// TODO: create dotother config folder to store paths and type/entry point info
        .host_config_path = host_config_path,
        .managed_asm_path = managed_asm_path,
        .dotnet_type = DO_STR("DotOther.Managed.DotOtherHost, DotOther.Managed"),
        .entry_point = DO_STR("EntryPoint"),

        // clang-format off
        .exception_callback = [](const dotother::NString message) {
          std::string  msg = message;
          OE_ERROR("C# Exception Caught : \n\t{}" , msg); 
        },
        // clang-format on
        .log_callback = &ManagedLoggingCallback,

        /// TODO: decide if DotOther should be responsible for invoking native methods or if it should be the responsibility of the user,
        ///         not really sure how to handle it in a generic way if it is handled by DotOther, but it would be nice to not have the user implement it,
        ///       another option is to simply use this hook as a filter that is always alled before InvokeNativeMethod
        // clang-format off
        .invoke_native_method_hook = [](uint64_t object_handle, const dotother::NString method_name) {
          std::string mname = method_name;
          OE_DEBUG("Invoking Native Method on object {:#08x}" , object_handle);
          OE_DEBUG(" > Method Name: {}" , mname);
          dotother::InteropInterface::Instance().InvokeNativeFunction(object_handle, mname); 
        },
        // clang-format on
        .internal_logging_hook = &NativeLoggingCallback,
      };

      host = dotother::Host::Instance(config);
      OE_ASSERT(host != nullptr, "Failed to create C# host");

      if (!host->LoadHost()) {
        OE_ERROR("Failed to load C# host");
        return false;
      } else {
        OE_DEBUG("Loaded C# Host");
      }

      host->CallEntryPoint();

      int32_t type_counter = 0;
      dotother::Interop().get_net_core_types(nullptr, &type_counter);

      DOTOTHER_LOG(DO_STR(" > Loading [{}] core types"), dotother::MessageLevel::TRACE, type_counter);

      std::vector<int32_t> type_ids;
      type_ids.resize(type_counter);
      dotother::Interop().get_net_core_types(type_ids.data(), &type_counter);

      for (auto id : type_ids) {
        // DOTOTHER_LOG(DO_STR(" > Loading core type with ID: {}"), dotother::MessageLevel::TRACE, id);

        Type type(id);
        Type* t = dotother::TypeCache::Instance().CacheType(std::forward<dotother::Type>(type));
        if (t == nullptr) {
          continue;
        }

        std::string name = t->FullName();
        ValueType vtype = GetValueTypeFromHash(FNV(name));
        if (vtype != ValueType::EMPTY_TYPE) {
          OE_DEBUG(" > Found core type [{} | {}]", name, vtype);
          core_type_map[vtype] = t->handle;
          value_type_map[t->handle] = vtype;
        }
      }

      load_success = true;

      OE_DEBUG("C# module initialized");
      return true;
    } catch (const std::exception& e) {
      OE_ERROR("Failed to create C# host : {}", e.what());
      return false;
    }
  }

  void CsModule::Shutdown() {
    if (host == nullptr) {
      OE_WARN("Attempting to shutdown C# module when it is not loaded");
      return;
    }

    dotother::GarbageCollector::Collect(-1, dotother::GCMode::DEFAULT, true, true);
    dotother::GarbageCollector::WaitForPendingFinalizers(-1);

    OE_DEBUG("Shutting down C# module");
    for (auto& [id, module] : loaded_modules) {
      module->Shutdown();
      module = nullptr;
    }
    loaded_modules.clear();

    dotother::TypeCache::Instance().Clear();

    OE_DEBUG("Unloading loaded C# assemblies");
    for (auto& [id, ctx] : assembly_contexts.contexts) {
      OE_DEBUG(" > Unloading assembly context {}", id);
      host->UnloadAssemblyContext(ctx);
    }

    assembly_contexts.contexts.clear();

    OE_DEBUG("Unloading C# assembly contexts");
    host->UnloadHost();
    host = nullptr;
    dotother::Host::Destroy();
  }

  void CsModule::Reload() {
    Shutdown();

    auto proj = AppState::ProjectContext();
    auto script_file = proj->GetMetadata().cs_project_file;
    OE_DEBUG("Kicking off build for C# scripts [{}]", script_file);

    if (!PlatformLayer::BuildProject(script_file)) {
      OE_ERROR("Failed to rebuild project scripts");
      return;
    }

    // auto editor_file = proj->GetMetadata().cs_editor_project_file;
    // OE_DEBUG("Kicking off build for editor scripts {}", editor_file);
    // if (!PlatformLayer::BuildProject(editor_file)) {
    //   OE_ERROR("Failed to rebuild editor scripts");
    // }

    if (!Initialize()) {
      OE_ERROR("Failed to reload C# module");
      return;
    }

    const Path engine_core_dir = Filesystem::GetEngineCoreDir();
    const Path engine_bin = engine_core_dir / "bin" / "Debug";
    const Path cs_core = engine_bin / "OtherEngine-CsCore" / "net8.0" / "OtherEngine-CsCore.dll";

    Ref<FileHandle> cs_core_dll = Filesystem::GetFile(cs_core);
    OE_ASSERT(cs_core_dll != nullptr, "Failed to find C# core DLL : {}", cs_core.string());
    OE_ASSERT(cs_core_dll->Exists(), "Failed to find C# core DLL : {}", cs_core.string());

    LoadScriptModule({
      .name = "OtherEngine.CsCore",
      .handle = cs_core_dll,
    });

    for (auto& [id, module] : loaded_modules_data) {
      if (module.name != "OtherEngine.CsCore") {
        LoadScriptModule(module);
      }
    }
  }

  Ref<ScriptModule> CsModule::GetScriptModule(const std::string_view name) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded", name);
      return nullptr;
    }

    auto hash = IdFromName(name);
    return GetScriptModule(hash);
  }

  Ref<ScriptModule> CsModule::GetScriptModule(const UUID& id) {
    if (!load_success) {
      OE_WARN("Attempting to get script module {} when C# module is not loaded", id);
      return nullptr;
    }

    OE_TRACE("Attempting to find C# script module {}", id);
    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    return nullptr;
  }

  Ref<ScriptModule> CsModule::LoadScriptModule(const ScriptMetadata& module_info) {
    OE_ASSERT(host != nullptr, "Attempting to load script module when C# module is not loaded");
    OE_ASSERT(load_success, "Attempting to load script module when C# module is not loaded");

    OE_ASSERT(module_info.handle != nullptr, "Failed to load script module : {}", module_info.name);
    OE_ASSERT(module_info.handle->Exists(), "Failed to load script module : {}", module_info.name);

    UUID id = IdFromName(module_info.name);
    OE_DEBUG(" > CsModule::LoadScriptModule({}) => id = {}", module_info.name, id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_WARN("Script module {} already loaded", module_info.name);
      return loaded_modules[id];
    }

    Ref<FileHandle> file = module_info.handle;

    AssemblyContext ctx = host->CreateAsmContext(module_info.name);
    if (ctx.context_id == -1) {
      OE_ERROR("Failed to create C# core assembly context");
      return nullptr;
    } else {
      OE_DEBUG(" > Created C# assembly context {} [{}]", module_info.name, ctx.context_id);
    }

    Path real_path = file->AbsolutePath();
    assembly_contexts.assembly_ids[id] = ctx.context_id;
    assembly_contexts.contexts[ctx.context_id] = ctx;
    OE_DEBUG(" > ctx[{}].LoadAssembly({}) [{}]", module_info.name, file->ProjectRelativePath(), id);

    /// we have to give up the resource here for a second, but we will get it back
    file->Close();

    ref<Assembly> assembly = nullptr;
    assembly = ctx.LoadAssembly(file->AbsolutePath().string());
    if (assembly == nullptr) {
      OE_ERROR("Failed to load C# assembly {} [{}]", module_info.name, id);

      file->Open();
      if (!file->IsOpen()) {
        OE_ERROR("File may be corrupt : {}", real_path.string());
      }
      return nullptr;
    } else {
      OE_DEBUG(" > Loaded C# assembly {} [{}]", module_info.name, id);
    }

    OE_TRACE("C# Assembly {} loaded [{}]", module_info.name, id);
    auto& m = loaded_modules[id] = NewRef<CsScript>(this, module_info.name, assembly);
    m->Initialize();
    loaded_modules_data[id] = module_info;

    file->Open();
    OE_ASSERT(file->IsOpen(), "Failed to reopen file : {}", real_path.string());
    OE_DEBUG(" > C# Script Loaded : {} [assembly : {}]", module_info.name, id);
    return m;
  }

  void CsModule::UnloadScript(const std::string& name) {
    UUID id = IdFromName(name);
    OE_DEBUG("Unloading C# script module {} [{}]", name, id);

    /// remove engine data
    auto itr = loaded_modules.find(id);
    if (itr != loaded_modules.end()) {
      itr->second->Shutdown();  /// this one will garbage collect and compact
      loaded_modules.erase(itr);
    }

    dotother::GarbageCollector::Collect(-1, dotother::GCMode::DEFAULT, true, false);  /// dont compact

    /* DotOther Code */ {
      /// find the DotOther ID
      auto itr = assembly_contexts.assembly_ids.find(id);
      if (itr == assembly_contexts.assembly_ids.end()) {
        OE_ERROR("Attempting to unload script module {} that is not loaded", name);
        return;
      }

      auto& [_, ctx_id] = *itr;

      /// find the DotOther context
      auto itr2 = assembly_contexts.contexts.find(ctx_id);
      if (itr2 == assembly_contexts.contexts.end()) {
        OE_ERROR("assembly ID [{}] is corrupt", name);
        return;
      }

      /* Unregistering from DotOther */ {
        /// unload this context
        auto& [__, ctx] = *itr2;
        host->UnloadAssemblyContext(ctx);
        dotother::GarbageCollector::Collect(ctx_id, dotother::GCMode::DEFAULT, true, false);  /// dont compact

        /// erase all DotOther data
        assembly_contexts.contexts.erase(itr2);
        assembly_contexts.assembly_ids.erase(itr);
      }
    }
  }

  void CsModule::UnloadAll() {
    for (auto& [_, m] : loaded_modules) {
      m->Shutdown();
    }
    loaded_modules.clear();

    dotother::GarbageCollector::Collect(-1, dotother::GCMode::DEFAULT, true, false);

    for (auto& [_, ctx_id] : assembly_contexts.assembly_ids) {
      auto itr2 = assembly_contexts.contexts.find(ctx_id);
      if (itr2 == assembly_contexts.contexts.end()) {
        continue;
      }

      auto& [__, ctx] = *itr2;
      host->UnloadAssemblyContext(ctx);
      dotother::GarbageCollector::Collect(ctx_id, dotother::GCMode::DEFAULT, true, false);
    }

    assembly_contexts.assembly_ids.clear();
    assembly_contexts.contexts.clear();
  }

  UUID CsModule::IdFromName(const std::string_view name) const {
    // std::string case_insensitive_name;
    // std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    return FNV(name);
  }

}  // namespace other
