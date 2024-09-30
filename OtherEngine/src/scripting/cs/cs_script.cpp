/**
 * \file scripting/cs/cs_script.cpp
 **/
#include "scripting/cs/cs_script.hpp"

#include "scripting/cs/cs_object.hpp"
#include "scripting/script_module.hpp"

namespace other {

  void CsScript::Initialize() {
    valid = assembly != nullptr;
  }

  void CsScript::Shutdown() {
    if (!valid) {
      return;
    }

    valid = false;
  }

  void CsScript::Reload() {
  }

  bool CsScript::HasScript(UUID id) const {
    if (!valid) {
      return false;
    }

    return false;
  }
      
  bool CsScript::HasScript(const std::string_view name , const std::string_view nspace) const {
    if (!valid) {
      return false;
    }

    auto itr = objects.find(FNV(fmtstr("{}.{}" , nspace , name)));
    if (itr != objects.end()) {
      return true;
    }

    return assembly->HasType(name , nspace);
  }
  
  Ref<ScriptObject> CsScript::GetScriptObject(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }

    UUID id = FNV(fmtstr("{}.{}" , nspace , name));
    if (loaded_objects.find(id) != loaded_objects.end()) {
      Ref<ScriptObjectHandle<CsObject>> obj = loaded_objects[id];
      if (obj != nullptr) {
        return obj.Raw();
      }
    }

    // retrieve type from assembly
    std::string asm_name = assembly->GetAsmQualifiedName(name , nspace);
    Type& type = assembly->GetType(asm_name);
    if (type.handle == -1) {
      OE_ERROR("Failed to retrieve type {} [{}]" , asm_name , assembly->Name());
      return nullptr;
    }

    OE_DEBUG("Retrieved type {} [{}]" , type.FullName() , type.handle);

    Ref<ScriptObjectHandle<CsObject>> obj = NewRef<CsObject>(this , type , id);
    objects[id] = obj;
    loaded_objects[id] = obj;

    obj->InitializeScriptFields();
    obj->InitializeScriptMethods();
    obj->OnBehaviorLoad();

    return obj.Raw();
  }

  std::vector<ScriptObjectTag> CsScript::GetObjectTags() {
   return {};
  }

} // namespace other
