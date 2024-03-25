/**
 * \file cs_script.cpp
 **/
#include "cs_script.hpp"

#include <mono/metadata/assembly.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"

namespace other {

  void CsScript::Initialize() {
    if (!Filesystem::FileExists(assembly_path)) {
      LogError("Mono Script Module path does not exist : {}" , assembly_path);
      return;
    }

    std::vector<char> file_data = Filesystem::ReadFileAsChars(assembly_path);

    MonoImageOpenStatus status;
    assembly_image = mono_image_open_from_data_full(
      file_data.data() , static_cast<uint32_t>(file_data.size()) , 
      true , &status , false
    );

    if (status != MONO_IMAGE_OK) {
      LogError("Mono Script Module image could not open : {}" , assembly_path);
      return;
    }

    assembly = mono_assembly_load_from_full(assembly_image , assembly_path.c_str() , &status , false);

    if (status != MONO_IMAGE_OK) {
      LogError("Mono Script Module assembly could not be loaded : {}" , assembly_path);
      return;
    }

    valid = true;

    LogDebug("Mono Script Module loaded : {}" , assembly_path);
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
      LogError("Mono object could not be created : {}" , name);
      return nullptr;
    }

    mono_runtime_object_init(object);

    loaded_objects[id] = CsObject(name , klass , object);
    loaded_objects[id].InitializeScriptMethods();

    return &loaded_objects[id];
  }

  MonoClass* CsScript::GetClass(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }

    LogDebug("Loading Mono class : {}" , name);

    UUID id = FNV(name);
    if (classes.find(id) != classes.end()) {
      MonoClass* klass = classes[id];
      if (klass != nullptr) {
        return klass;
      }
    }

    MonoClass* klass = mono_class_from_name(assembly_image , nspace.c_str() , name.c_str());
    if (klass == nullptr) {
      LogError("Mono class not found : {}" , name);
      return nullptr;
    } else {
      LogInfo("Mono class {} loaded from assembly : {}" , name , assembly_path);
    }

    return klass;
  }

} // namespace other
