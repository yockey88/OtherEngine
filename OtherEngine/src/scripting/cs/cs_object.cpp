/**
 * \file scripting/cs/cs_object.cpp
*/
#include "scripting/cs/cs_object.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "core/logger.hpp"
#include "scripting/cs/mono_utils.hpp"
#include "scripting/cs/cs_script_cache.hpp"

namespace other {
      
  CsCache* CsObject::GetScriptCache() const {
    return script_cache;
  }

  void CsObject::InitializeScriptMethods() {
    obj_vtable = mono_object_get_vtable(instance);

    constexpr auto load_hash = FNV("OnBehaviorLoad"); 
    constexpr auto init_hash = FNV("OnInitialize"); 
    constexpr auto shutd_hash = FNV("OnShutdown"); 
    constexpr auto unload_hash = FNV("OnBehaviorUnload"); 
    constexpr auto start_hash = FNV("OnStart"); 
    constexpr auto stop_hash = FNV("OnStop"); 
    constexpr auto update_hash = FNV("Update"); 
    constexpr auto render_hash = FNV("RenderObject"); 
    constexpr auto ui_hash = FNV("RenderUI"); 

    auto load_itr = type_data->methods.find(load_hash);
    auto init_itr = type_data->methods.find(init_hash);
    auto shutd_itr = type_data->methods.find(shutd_hash);
    auto unload_itr = type_data->methods.find(unload_hash);
    auto start_itr = type_data->methods.find(start_hash);
    auto stop_itr = type_data->methods.find(stop_hash);
    auto update_itr = type_data->methods.find(update_hash);
    auto render_itr = type_data->methods.find(render_hash);
    auto ui_itr = type_data->methods.find(ui_hash);

    if (load_itr != type_data->methods.end()) {
      on_load_method = mono_object_get_virtual_method(instance , load_itr->second.asm_method); 
    }

    if (init_itr != type_data->methods.end()) {
      initialize_method = mono_object_get_virtual_method(instance , init_itr->second.asm_method);
    }
    
    if (shutd_itr != type_data->methods.end()) {
      shutdown_method = mono_object_get_virtual_method(instance , shutd_itr->second.asm_method);
    }
    
    if (unload_itr != type_data->methods.end()) {
      on_unload_method = mono_object_get_virtual_method(instance , unload_itr->second.asm_method); 
    }
    
    if (start_itr != type_data->methods.end()) {
      on_start = mono_object_get_virtual_method(instance , start_itr->second.asm_method);
    }
    
    if (stop_itr != type_data->methods.end()) {
      on_stop = mono_object_get_virtual_method(instance , stop_itr->second.asm_method);
    }
    
    if (update_itr != type_data->methods.end()) {
      update_method = mono_object_get_virtual_method(instance , update_itr->second.asm_method);
    }

    if (render_itr != type_data->methods.end()) {
      render_method = mono_object_get_virtual_method(instance , render_itr->second.asm_method);
    }

    if (ui_itr != type_data->methods.end()) {
      render_ui_method = mono_object_get_virtual_method(instance , ui_itr->second.asm_method);
    }
  }

  void CsObject::InitializeScriptFields() {
    for (auto& [fid , f] : type_data->fields) {
      if (f.HasFlag(FieldAccessFlag::PRIVATE) || f.HasFlag(FieldAccessFlag::PROTECTED)) {
        continue;
      }

      auto& fval = fields[fid] = ScriptField{};
      fval.name = f.name;

      Opt<Value> val = std::nullopt;

      if (f.is_property) {
        f.asm_property_getter = mono_object_get_virtual_method(instance , f.asm_property_getter);
        f.asm_property_setter = mono_object_get_virtual_method(instance , f.asm_property_setter);

        val = GetMonoProperty(f.asm_property_getter , f.IsArray());
      } else {
        val = GetMonoField(f.asm_field , f.IsArray());
      }

      if (val.has_value()) {
        fval.value = val.value();
      }

      fval.bounds = GetMonoBoundsAttribute(f , script_cache);
    }
  } 

  void CsObject::UpdateNativeFields() {
    for (auto& [fid , f] : type_data->fields) {
      if (f.HasFlag(FieldAccessFlag::PRIVATE) || f.HasFlag(FieldAccessFlag::PROTECTED)) {
        continue;
      }

      auto& fval = fields[fid];

      Opt<Value> val = std::nullopt;
      if (f.is_property) {
        val = GetMonoProperty(f.asm_property_getter , f.IsArray());
      } else {
        val = GetMonoField(f.asm_field , f.IsArray());
      }

      if (val.has_value()) {
        fval.value = val.value();
      }
    }
  }
      
  Opt<Value> CsObject::GetField(const std::string& name) {
    UUID id = FNV(name);
    auto field_data = type_data->fields.find(id);
    if (field_data == type_data->fields.end()) {
      OE_ERROR("Failed to get field {} from C# script {}" , name , script_name);
      return std::nullopt;
    }

    if (field_data->second.is_property) {
      return GetMonoProperty(field_data->second.asm_property_getter , field_data->second.IsArray());
    } else {
      return GetMonoField(field_data->second.asm_field , field_data->second.IsArray());
    }
  }

  void CsObject::SetField(const std::string& name , const Value& value) {
    UUID id = FNV(name);
    auto field = type_data->fields.find(id);
    if (field == type_data->fields.end()) {
      OE_ERROR("Attempting to set non-existent field {} on C# script {}" , name , script_name);
      return;
    }

    auto& fdata = field->second;

    Value real_value = value;
    if (HasBoundsAttribute(fdata)) {
      real_value = ClampValue(fdata , script_cache , value);
    } 

    if (fdata.is_property) {
      SetMonoProperty(fdata.asm_property_setter , real_value , fdata.IsArray());
    } else {
      SetMonoField(fdata.asm_field , real_value , fdata.IsArray());
    }
  }
    
  void CsObject::OnBehaviorLoad() {
    if (on_load_method != nullptr) {
      CallMonoMethod(on_load_method);
    }
  }

  void CsObject::Initialize() {
    if (initialize_method != nullptr) {
      CallMonoMethod(initialize_method);
    }

    is_initialized = true;
  }
  
  void CsObject::Shutdown() {
    if (shutdown_method != nullptr) {
      CallMonoMethod(shutdown_method);
    }

    is_initialized = false;
    fields.clear();
  }
  
  void CsObject::OnBehaviorUnload() {
    if (on_unload_method != nullptr) {
      CallMonoMethod(on_unload_method);
    }
  }

  void CsObject::Start() {
    if (on_start != nullptr) {
      CallMonoMethod(on_start);
    }
  }

  void CsObject::Stop() {
    if (on_stop != nullptr) {
      CallMonoMethod(on_stop);
    }
  }

  void CsObject::Update(float dt) {
    if (update_method != nullptr) {
      Parameter arg = {
        .handle = &dt ,
        .type = ValueType::FLOAT ,
      };
      CallMonoMethod(update_method , 1 , &arg);
    }
  }

  void CsObject::Render() {
    if (render_method != nullptr) {
      CallMonoMethod(render_method);
    }
  }

  void CsObject::RenderUI() {
    if (render_ui_method != nullptr) {
      CallMonoMethod(render_ui_method);
    }
  }
  
  Opt<Value> CsObject::GetMonoField(MonoClassField* field , bool is_array) {
    MonoObject* mobj = mono_field_get_value_object(app_domain , field , instance);
    if (mobj == nullptr) {
      OE_ERROR("Failed to retrieve C# field!");
      return std::nullopt;
    }

    return MonoObjectToValue(mobj , field , is_array);
  }

  Opt<Value> CsObject::GetMonoProperty(MonoMethod* getter , bool is_array) {
    MonoMethod* vgetter = mono_object_get_virtual_method(instance , getter);
    if (vgetter == nullptr) {
      OE_ERROR("failed to find C# property getter!");
      return std::nullopt;;
    }

    return CallMonoMethod(vgetter);
  }

  void CsObject::SetMonoField(MonoClassField* field , const Value& value , bool is_array) {
    if (value.Type() == ValueType::STRING) {
      std::string val = value.Get<std::string>();
      MonoString* mstr = mono_string_new(app_domain , val.c_str());
      mono_field_set_value(instance , field , mstr);
    } else {
      mono_field_set_value(instance , field , value.AsRawMemory());
    }
  }

  void CsObject::SetMonoProperty(MonoMethod* setter , const Value& value , bool is_array) {
    Parameter param {
      .handle = value.AsRawMemory() ,
      .type = value.Type() ,
    };

    if (param.type == ValueType::STRING) {
      std::string string = value.Get<std::string>(); 
      MonoString* mstr = mono_string_new(app_domain , string.c_str());

      param.handle = mstr;
    }

    MonoMethod* vsetter = mono_object_get_virtual_method(instance , setter);
    if (vsetter == nullptr) {
      OE_ERROR("failed to find C# property setter!");
      return;
    }

    CallMonoMethod(vsetter , 1 , &param);
  }
  
  Opt<Value> CsObject::CallMonoMethod(MonoMethod* method , uint32_t argc , Parameter* args) {
    if (is_corrupt) {
      return std::nullopt;
    }

    if (instance == nullptr) {
      OE_ERROR("CsObject::CallMethod: instance is nullptr");
      is_corrupt = true;
      return std::nullopt;
    }

    if (method == nullptr) {
      return std::nullopt;
    }

    std::vector<ParamHandle> params(argc);
    for (uint32_t i = 0; i < argc; ++i) {
      params[i] = args[i].handle;
    }

    MonoObject* exception = nullptr;
    MonoObject* ret_val = nullptr;

    ret_val = mono_runtime_invoke(method , instance , params.data() , &exception);

    if (exception) {
      OE_ERROR("C# Exception thrown by : {}" , script_name);
      CheckMonoError();
      return std::nullopt;
    }

    if (ret_val == nullptr) {
      return std::nullopt;
    }

    return MonoObjectToValue(ret_val);
  }

  void CsObject::OnSetEntityId() {
    MonoProperty* property = mono_class_get_property_from_name(type_data->asm_class, "ObjectID");
    MonoMethod* set_entity_id = mono_property_get_set_method(property);
    set_entity_id = mono_object_get_virtual_method(instance , set_entity_id);

    uint64_t id = entity_id.Get();
    Parameter p{
      .handle = &id ,
      .type = ValueType::UINT64 ,
    };

    CallMonoMethod(set_entity_id , 1 , &p);
  }
  
  Opt<Value> CsObject::OnCallMethod(const std::string_view name , std::span<Value> args) {
    /// dont need to construct the vector below if no args provided
    if (args.size() == 0) {
      return OnCallMethod(name , nullptr , 0);
    }

    std::vector<Parameter> call_args(args.size());
    for (uint32_t i = 0; i < args.size(); ++i) {
      call_args[i] = {
        args[i].AsRawMemory() ,
        args[i].Type() ,
      };
    }

    return OnCallMethod(name , call_args.data() , call_args.size());
  }
  
  Opt<Value> CsObject::OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc)  {
    UUID id = FNV(name);
    for (const auto& [mid , m] : type_data->methods) {
      if (id == mid) {
        return CallMonoMethod(m.asm_method , argc , args);
      }
    }

    OE_ERROR("Failed to find method {} on C# script {}" , name , script_name);
    return std::nullopt;
  }

} // namespace other
