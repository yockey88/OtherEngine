/**
 * \file scripting/cs/cs_script.cpp
 **/
#include "scripting/cs/cs_script.hpp"

#include <mono/metadata/assembly.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/image.h>
#include <mono/metadata/row-indexes.h>

#include "core/filesystem.hpp"
#include "core/platform.hpp"
#include "application/app.hpp"
#include "scripting/cs/cs_garbage_collector.hpp"
#include "scripting/script_engine.hpp"

namespace other {
      
  MonoImage* CsScript::GetImage() const {
    return assembly_image;
  }

  MonoAssembly* CsScript::GetAsm() const {
    return assembly;
  }

  void CsScript::Initialize() {
    if (!Filesystem::FileExists(assembly_path)) {
      OE_ERROR("Mono Script Module path does not exist : {}" , assembly_path);
      return;
    }

    std::vector<char> file_data = Filesystem::ReadFileAsChars(assembly_path);

    MonoImageOpenStatus status;
    assembly_image = mono_image_open_from_data_full(file_data.data() , static_cast<uint32_t>(file_data.size()) , 
                                               true , &status , false);

    if (status != MONO_IMAGE_OK) {
      OE_ERROR("Mono Script Module image could not open : {}" , assembly_path);
      return;
    }

    assembly = mono_assembly_load_from_full(assembly_image , assembly_path.c_str() , &status , false);

    if (status != MONO_IMAGE_OK) {
      OE_ERROR("Mono Script Module assembly could not be loaded : {}" , assembly_path);
      return;
    }

    OE_INFO("Loaded C# Assembly : {} [{}]" , assembly_path , script_id);

    const MonoTableInfo* asm_details = mono_image_get_table_info(assembly_image , MONO_TABLE_ASSEMBLY);
    uint32_t cols[MONO_TABLE_ASSEMBLY];
    mono_metadata_decode_row(asm_details , 0 , cols , MONO_ASSEMBLY_SIZE);

    std::string asm_name = mono_metadata_string_heap(assembly_image , cols[MONO_ASSEMBLY_NAME]);
    uint32_t major_version = cols[MONO_ASSEMBLY_MAJOR_VERSION] > 0 ?
      cols[MONO_ASSEMBLY_MAJOR_VERSION] : 1;
    uint32_t minor_version = cols[MONO_ASSEMBLY_MINOR_VERSION];
    uint32_t patch_version = cols[MONO_ASSEMBLY_BUILD_NUMBER];
    uint32_t revision_version = cols[MONO_ASSEMBLY_REV_NUMBER];

    OE_INFO(" > Script assembly loaded : {} [{}.{}.{}.{}]" , 
            asm_name , major_version , minor_version , patch_version , revision_version);

    const MonoTableInfo* ref_asms = mono_image_get_table_info(assembly_image , MONO_TABLE_ASSEMBLYREF);
    uint32_t rows = mono_table_info_get_rows(ref_asms);

    for (uint32_t i = 0; i < rows; ++i) {
      uint32_t cols[MONO_ASSEMBLYREF_SIZE];
      mono_metadata_decode_row(ref_asms , i , cols , MONO_ASSEMBLYREF_SIZE);

      std::string name = mono_metadata_string_heap(assembly_image , cols[MONO_ASSEMBLYREF_NAME]);
      major_version = cols[MONO_ASSEMBLYREF_MAJOR_VERSION] > 0 ?
        cols[MONO_ASSEMBLYREF_MAJOR_VERSION] : 1;
      minor_version = cols[MONO_ASSEMBLYREF_MINOR_VERSION];
      patch_version = cols[MONO_ASSEMBLYREF_BUILD_NUMBER];
      revision_version = cols[MONO_ASSEMBLYREF_REV_NUMBER];

      OE_INFO("  > {} references assembly : {} [{}.{}.{}.{}]" ,
              asm_name , name , major_version , minor_version , patch_version , revision_version);
    }

    App* app_ctx = ScriptEngine::GetAppContext();

    /// since script-modules loaded from the .dll path and not the actual script we have to find the 
    ///   original C# file to track with the filewatcher
    if (assembly_path.find("OtherEngine-CsCore") == std::string::npos) {
      Path script_dir = app_ctx->GetProjectContext()->GetMetadata().assets_dir / "scripts";
      Path real_script_file_full = script_dir / Path{ assembly_path }.filename();
      std::string real_file = real_script_file_full.string().substr(0 , real_script_file_full.string().find_last_of('.'));
      real_file += ".cs";
      SetPath(real_file);
    }
    
    valid = true;

    OE_DEBUG("Mono Script Module loaded : {}" , assembly_path);
  }

  void CsScript::Shutdown() {
    if (!valid) {
      return;
    }

    for (const auto& [id , gch] : gc_handles) {
      CsGarbageCollector::FreeHandle(gch);
    }

    cached_symbols.Clear();

    for (auto& [id , obj] : loaded_objects) {
      obj.Shutdown();
    }
    loaded_objects.clear();
    classes.clear();

    mono_image_close(assembly_image);
    assembly_image = nullptr;

    valid = false;
  }

  void CsScript::Reload() {
    App* app_ctx = ScriptEngine::GetAppContext();

    Path project_dir = app_ctx->GetProjectContext()->GetMetadata().file_path.parent_path();
    Path build_file = project_dir / Path{ assembly_path }.filename();
    std::string real_file = build_file.string().substr(0 , build_file.string().find_last_of('.'));
    real_file += ".csproj";

    if (!Filesystem::FileExists(real_file)) {
      return;
    }

    OE_DEBUG("Kicking off build for script {}" , assembly_path);

    if (!PlatformLayer::BuildProject(real_file)) {
      OE_ERROR("Failed to rebuild project file");
    }
  }
      
  ScriptObject* CsScript::GetScript(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }

    UUID id = FNV(name);
    if (loaded_objects.find(id) != loaded_objects.end()) {
      CsObject* object = &loaded_objects[id];
      if (object != nullptr) {
        return object;
      }
    }

    OE_DEBUG("Retrieving {} from {}" , name , assembly_path);

    MonoClass* klass = GetClass(name , nspace);
    if (klass == nullptr) {
      return nullptr;
    }

    MonoObject* object = mono_object_new(app_domain , klass);
    if (object == nullptr) {
      OE_ERROR("Mono object could not be created : {}" , name);
      return nullptr;
    }

    mono_runtime_object_init(object);

    UUID class_id = FNV(name);
    CsTypeData* type_data = cached_symbols.GetClassData(class_id);
    
    GcHandle gc_handle = CsGarbageCollector::NewHandle(object);

    loaded_objects[id] = CsObject(module_name , name , type_data , object , assembly_image , 
                                  app_domain , class_id , &cached_symbols , nspace);
    loaded_objects[id].InitializeScriptMethods();
    loaded_objects[id].InitializeScriptFields();

    loaded_symbols[id] = {
      .name_space = nspace , 
      .name = name ,
    };

    gc_handles[id] = gc_handle;

    return &loaded_objects[id];
  }

  MonoClass* CsScript::GetClass(const std::string& name , const std::string& nspace) {
    if (!valid) {
      OE_WARN("Attempting to retrieve script {} from invalid assembly {}" , name , assembly_path);
      return nullptr;
    }

    OE_DEBUG("Loading Mono class : {}" , name);

    UUID id = FNV(name);
    if (classes.find(id) != classes.end()) {
      MonoClass* klass = classes[id];
      if (klass != nullptr) {
        return klass;
      }
    }

    OE_DEBUG("Looking for {} in {}" , name , assembly_path);

    MonoClass* klass = mono_class_from_name(assembly_image , nspace.c_str() , name.c_str());
    if (klass == nullptr) {
      OE_ERROR("Mono class not found : {}" , name);
      return nullptr;
    } else {
      OE_INFO("Mono class {} loaded from assembly : {}" , name , assembly_path);
    }

    classes[id] = klass;
    cached_symbols.CacheClass(name , klass);

    return klass;
  }

} // namespace other
