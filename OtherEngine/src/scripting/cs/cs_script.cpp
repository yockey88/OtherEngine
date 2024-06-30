/**
 * \file scripting/cs/cs_script.cpp
 **/
#include "scripting/cs/cs_script.hpp"

#include <mono/metadata/assembly.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/class.h>
#include <mono/metadata/image.h>
#include <mono/metadata/object.h>
#include <mono/metadata/row-indexes.h>

#include "core/filesystem.hpp"

#include "application/app.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/cs/cs_garbage_collector.hpp"

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
    loaded_objects.clear();
    classes.clear();

    mono_image_close(assembly_image);
    assembly_image = nullptr;

    valid = false;
  }

  void CsScript::Reload() {
  }

  bool CsScript::HasScript(UUID id) {
    if (!valid) {
      return false;
    }

    for (const auto& [cid , type] : cached_symbols.class_data) {
      if (id == cid) {
        return true;
      }
    }
    
    const MonoTableInfo* classes = mono_image_get_table_info(assembly_image , MONO_TABLE_TYPEDEF);
    const uint32_t num_rows = mono_table_info_get_rows(classes);

    for (auto i = 0; i < num_rows; ++i) {
      uint32_t cols[MONO_TYPEDEF_SIZE];
      mono_metadata_decode_row(classes , i , cols , MONO_TYPEDEF_SIZE);

      std::string sname = mono_metadata_string_heap(assembly_image , cols[MONO_TYPEDEF_NAME]);
      if (sname == "<Module>") {
        continue;
      }

      if (id.Get() == FNV(sname)) {
        return true; 
      } 
    }

    return false;
  }

      
  bool CsScript::HasScript(const std::string_view name , const std::string_view nspace) {
    if (!valid) {
      return false;
    }

    for (const auto& [id , type] : cached_symbols.class_data) {
      if (!nspace.empty() && name == type.name && nspace == type.name_space) {
        return true;
      } else if (name == type.name) {
        return true;
      }
    }
    
    const MonoTableInfo* classes = mono_image_get_table_info(assembly_image , MONO_TABLE_TYPEDEF);
    const uint32_t num_rows = mono_table_info_get_rows(classes);

    for (auto i = 0; i < num_rows; ++i) {
      uint32_t cols[MONO_TYPEDEF_SIZE];
      mono_metadata_decode_row(classes , i , cols , MONO_TYPEDEF_SIZE);

      std::string sname = mono_metadata_string_heap(assembly_image , cols[MONO_TYPEDEF_NAME]);
      if (sname == "<Module>") {
        continue;
      }
      std::string sname_space = mono_metadata_string_heap(assembly_image , cols[MONO_TYPEDEF_NAMESPACE]);

      if (name == sname && sname_space.empty() && nspace.empty()) {
        return true; 
      } else if (name == sname && !sname_space.empty() && !nspace.empty()) {
        return sname_space == nspace; 
      }
    }

    return false;
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

    auto& obj = loaded_objects[id] = CsObject(module_name , name , type_data , object , assembly_image , 
                                              app_domain , class_id , &cached_symbols , nspace);
    obj.InitializeScriptMethods();
    obj.InitializeScriptFields();

    loaded_symbols[id] = {
      .name_space = nspace , 
      .name = name ,
    };

    gc_handles[id] = gc_handle;

    return &loaded_objects[id];
  }

  std::vector<ScriptObjectTag> CsScript::GetObjectTags() {
    auto app_ctx = ScriptEngine::GetAppContext();
    auto& proj = app_ctx->GetProjectContext()->GetMetadata();

    std::vector<ScriptObjectTag> object_names;
    const MonoTableInfo* classes = mono_image_get_table_info(assembly_image , MONO_TABLE_TYPEDEF);
    const uint32_t num_rows = mono_table_info_get_rows(classes);

    for (auto i = 0; i < num_rows; ++i) {
      uint32_t cols[MONO_TYPEDEF_SIZE];
      mono_metadata_decode_row(classes , i , cols , MONO_TYPEDEF_SIZE);

      std::string full_name;

      std::string name = mono_metadata_string_heap(assembly_image , cols[MONO_TYPEDEF_NAME]);
      if (name == "<Module>") {
        continue;
      }

      full_name = name;

      std::string name_space = mono_metadata_string_heap(assembly_image , cols[MONO_TYPEDEF_NAMESPACE]);
      if (!name_space.empty()) {
        full_name = name_space + "::" + name;
      }

      Path scripts_path = proj.assets_dir / "scripts";
      Path editor_path = proj.assets_dir / "editor";

      Path file_path = scripts_path / (name + ".cs");
      Path editor_file_path = editor_path = editor_path / (name + "cs");
      Path actual_path;

      if (Filesystem::FileExists(file_path)) {
        cs_files.push_back(file_path);
        actual_path = file_path;
      } else if (Filesystem::FileExists(editor_file_path)) {
        editor_files.push_back(editor_file_path);
        actual_path = editor_file_path;
      }

      object_names.push_back(ScriptObjectTag{
        .object_id = FNV(name) ,
        .name = name , 
        .nspace = name_space ,
        .path = (actual_path.filename() == (name + ".cs")) ?
          actual_path.string() : "",
        .lang_type = language ,
      });
    }

    return object_names;
  }

  void CsScript::LoadCsFiles() {
    cs_files.clear();
    editor_files.clear();

    auto app_ctx = ScriptEngine::GetAppContext();
    auto& proj = app_ctx->GetProjectContext()->GetMetadata();

    Path scripts_dir = proj.assets_dir / "scripts";
    Path editor_dir = proj.assets_dir / "editor";

    for (auto& entry : std::filesystem::recursive_directory_iterator(scripts_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".cs") {
        cs_files.push_back(entry.path());
      } 
    }
    
    for (auto& entry : std::filesystem::recursive_directory_iterator(editor_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".cs") {
        editor_files.push_back(entry.path());
      } 
    }
  }

  MonoClass* CsScript::GetClass(const std::string& name , const std::string& nspace) {
    if (!valid) {
      OE_WARN("Attempting to retrieve C# script {} from invalid assembly {}" , name , assembly_path);
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

    OE_DEBUG("Looking for {}::{} in {}" , nspace , name , assembly_path);

    MonoClass* klass = mono_class_from_name(assembly_image , nspace.c_str() , name.c_str());
    if (klass == nullptr) {
      OE_ERROR("Mono class not found : {}" , name);
      return nullptr;
    } 

    mono_class_init(klass);

    MonoVTable* vtable = mono_class_vtable(app_domain , klass);
    mono_runtime_class_init(vtable);

    classes[id] = klass;
    cached_symbols.CacheClass(name , klass);

    OE_INFO("Mono class {} loaded from assembly : {}" , name , assembly_path);
    return klass;
  }

} // namespace other
