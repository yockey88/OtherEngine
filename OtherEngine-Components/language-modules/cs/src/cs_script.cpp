/**
 * \file cs_script.cpp
 **/
#include "cs_script.hpp"

#include <mono/metadata/assembly.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/image.h>
#include <mono/metadata/row-indexes.h>

#include "core/filesystem.hpp"

namespace other {

  void CsScript::Initialize() {
    if (!Filesystem::FileExists(assembly_path)) {
      OE_ERROR("Mono Script Module path does not exist : {}" , assembly_path);
      return;
    }

    std::vector<char> file_data = Filesystem::ReadFileAsChars(assembly_path);

    MonoImageOpenStatus status;
    assembly_image = mono_image_open_from_data_full(
      file_data.data() , static_cast<uint32_t>(file_data.size()) , 
      true , &status , false
    );

    if (status != MONO_IMAGE_OK) {
      OE_ERROR("Mono Script Module image could not open : {}" , assembly_path);
      return;
    }

    assembly = mono_assembly_load_from_full(assembly_image , assembly_path.c_str() , &status , false);

    if (status != MONO_IMAGE_OK) {
      OE_ERROR("Mono Script Module assembly could not be loaded : {}" , assembly_path);
      return;
    }

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

      OE_INFO("  > {} references asembly : {} [{}.{}.{}.{}]" ,
              asm_name , name , major_version , minor_version , patch_version , revision_version);
    }

    valid = true;

    OE_DEBUG("Mono Script Module loaded : {}" , assembly_path);
  }

  void CsScript::Shutdown() {
    if (!valid) {
      return;
    }

    mono_image_close(assembly_image);
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

    loaded_objects[id] = CsObject(name , klass , object , assembly_image);
    loaded_objects[id].InitializeScriptMethods();

    return &loaded_objects[id];
  }

  MonoClass* CsScript::GetClass(const std::string& name , const std::string& nspace) {
    if (!valid) {
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

    MonoClass* klass = mono_class_from_name(assembly_image , nspace.c_str() , name.c_str());
    if (klass == nullptr) {
      OE_ERROR("Mono class not found : {}" , name);
      return nullptr;
    } else {
      OE_INFO("Mono class {} loaded from assembly : {}" , name , assembly_path);
    }

    return klass;
  }

} // namespace other
