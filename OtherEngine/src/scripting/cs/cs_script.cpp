/**
 * \file scripting/cs/cs_script.cpp
 **/
#include "scripting/cs/cs_script.hpp"

#include <hosting/garbage_collector.hpp>

#include "scripting/script_module.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/cs/cs_bindings.hpp"

namespace other {

  void CsScript::Initialize() {
    valid = assembly != nullptr;

    /// FIXME: this feels super hacky
    if (valid && module_name == "OtherEngine.CsCore") {
      cs_script_bindings::RegisterInternalCalls(assembly);
      assembly->UploadInternalCalls();
    }
  }

  void CsScript::Shutdown() {
    if (!valid) {
      return;
    }

    dotother::GarbageCollector::Collect(assembly->GetId() , dotother::GCMode::DEFAULT , true , true);
    dotother::GarbageCollector::WaitForPendingFinalizers(assembly->GetId());

    for (auto& [id , obj] : loaded_objects) {
      obj->OnBehaviorUnload();
      obj = nullptr;
    }
    loaded_objects.clear();

    valid = false;
  }

  void CsScript::Reload() {
  }

  bool CsScript::HasScript(UUID id) const {
    if (!valid) {
      return false;
    }

    return loaded_objects.find(id) != loaded_objects.end();
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

    OE_DEBUG("Getting C# object {}::{}" , nspace , name);
    UUID id = FNV(fmtstr("{}.{}" , nspace , name));

    if (auto itr = loaded_objects.find(id); itr != loaded_objects.end()) {
      Ref<ScriptObject> obj = itr->second;
      if (obj != nullptr) {
        OE_DEBUG(" > Retrieved C# object {}::{}" , nspace , name);
        return obj;
      }
    }

    // retrieve type from assembly
    std::string asm_name = assembly->GetAsmQualifiedName(name , nspace);
    OE_DEBUG(" > Looking for type {}::{} [{}]" , nspace , name , asm_name);
    
    Type& type = assembly->GetType(asm_name);
    if (type.handle == -1) {
      OE_ERROR("Failed to retrieve type {} [{}]" , asm_name , assembly->Name());
      return nullptr;
    } else {
      OE_DEBUG("  > Type found [{}]" , type.handle);
    }

    Ref<ScriptObjectHandle<CsObject>> obj = NewRef<CsObject>(this , type , id);
    Ref<ScriptObject>& ret = objects[id] = obj;
    loaded_objects[id] = obj;

    obj->InitializeScriptFields();
    obj->InitializeScriptMethods();
    obj->OnBehaviorLoad();

    return ret;
  }

  std::vector<ScriptObjectTag> CsScript::GetObjectTags() {
   return {};
  }

} // namespace other
