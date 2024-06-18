/**
 * \file scripting/cs/cs_object.cpp
*/
#include "scripting/cs/cs_object.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "core/logger.hpp"
#include "scripting/cs/mono_utils.hpp"
#include "scripting/cs/cs_script_cache.hpp"

namespace other {

  void CsObject::InitializeScriptMethods() {
    obj_vtable = mono_object_get_vtable(instance);

    MonoClass* parent = mono_class_get_parent(type_data->asm_class);
    
    initialize_method = mono_class_get_method_from_name(type_data->asm_class , "OnInitialize" , 0);
    shutdown_method = mono_class_get_method_from_name(type_data->asm_class , "OnShutdown" , 0);

    on_start = mono_class_get_method_from_name(type_data->asm_class , "OnStart" , 0);
    on_stop = mono_class_get_method_from_name(type_data->asm_class , "OnStop" , 0);

    update_method = mono_class_get_method_from_name(type_data->asm_class , "Update" , 1);
    render_method = mono_class_get_method_from_name(type_data->asm_class , "RenderObject" , 0);
    render_ui_method = mono_class_get_method_from_name(type_data->asm_class , "RenderUI" , 0);

    while (parent != nullptr) {
      if (initialize_method != nullptr && shutdown_method != nullptr &&
          on_start != nullptr && on_stop != nullptr &&
          update_method != nullptr && render_method != nullptr &&
          render_ui_method != nullptr) {
        break;
      }

      if (initialize_method == nullptr) {
        initialize_method = mono_class_get_method_from_name(parent , "OnInitialize" , 0);
      }
      if (shutdown_method == nullptr) {
        shutdown_method = mono_class_get_method_from_name(parent , "OnShutdown" , 0);
      }
      
      if (on_start == nullptr) {
        on_start = mono_class_get_method_from_name(parent , "OnStart" , 0);
      }
      if (on_stop == nullptr) {
        on_stop = mono_class_get_method_from_name(parent , "OnStop" , 0);
      }
      
      if (update_method == nullptr) {
        update_method = mono_class_get_method_from_name(parent , "Update" , 1);
      }
      if (render_method == nullptr) {
        render_method = mono_class_get_method_from_name(parent , "Render" , 0);
      }
      if (render_ui_method == nullptr) {
        render_ui_method = mono_class_get_method_from_name(parent , "RenderUI" , 0);
      }
      
      const char* name = mono_class_get_name(parent);
      if (std::string{ name } == "OtherObject") {
        break;  
      }

      parent = mono_class_get_parent(parent);
    }
  }

  void CsObject::InitializeScriptFields() {
    for (const auto& [fid , f] : type_data->fields) {
      if (f.HasFlag(FieldAccessFlag::PRIVATE) || f.HasFlag(FieldAccessFlag::PROTECTED)) {
        continue;
      }

      auto& fval = fields[fid] = ScriptField{};
      fval.name = f.name;

      Opt<Value> val = std::nullopt;

      if (f.is_property) {
        val = GetMonoProperty(f.asm_property_getter);
      } else {
        val = GetMonoField(f.asm_field);
      }

      if (val.has_value()) {
        fval.value = val.value();
      }
    }
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
      
  Opt<Value> CsObject::GetField(const std::string& name) {
    UUID id = FNV(name);
    for (const auto& [fid , f] : type_data->fields) {
      if (id == fid && f.is_property) {
        return GetMonoProperty(f.asm_property_getter);
      } else if (id == fid) {
        return GetMonoField(f.asm_field);
      }
    }
    
    OE_ERROR("Failed to find field {} on C# script {}" , name , script_name);
    return std::nullopt;
  }

  void CsObject::SetField(const std::string& name , const Value& value) {
    UUID id = FNV(name);
    for (const auto& [fid , f] : type_data->fields) {
      if (id == fid && f.is_property) {
        SetMonoProperty(f.asm_property_setter , value);
      } else if (id == fid) {
        SetMonoField(f.asm_field , value);
      }
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

      
  Opt<Value> CsObject::GetMonoField(MonoClassField* field) {
    MonoObject* mobj = mono_field_get_value_object(app_domain , field , instance);
    if (mobj == nullptr) {
      return std::nullopt;
    }

    return MonoObjectToValue(mobj);
  }

  Opt<Value> CsObject::GetMonoProperty(MonoMethod* getter) {
    MonoMethod* vgetter = mono_object_get_virtual_method(instance , getter);
    return CallMonoMethod(vgetter);
  }

  void CsObject::SetMonoField(MonoClassField* field , const Value& value) {
    if (value.Type() == ValueType::STRING) {
      std::string val = value.Get<std::string>();
      MonoString* mstr = mono_string_new(app_domain , val.c_str());
      mono_field_set_value(instance , field , mstr);
    } else {
      mono_field_set_value(instance , field , value.AsRawMemory());
    }
  }

  void CsObject::SetMonoProperty(MonoMethod* setter , const Value& value) {
    Parameter param {
      .handle = value.AsRawMemory() ,
      .type = value.Type() ,
    };
    MonoMethod* vsetter = mono_object_get_virtual_method(instance , setter);
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
    MonoMethod* vmethod = mono_object_get_virtual_method(instance , method);

    ret_val = mono_runtime_invoke(vmethod , instance , params.data() , &exception);

    if (exception) {
      OE_ERROR("C# Exception thrown by : {}" , script_name);
      CheckMonoError();
      is_corrupt = true; 
      return std::nullopt;
    }

    if (ret_val == nullptr) {
      return std::nullopt;
    }

    return MonoObjectToValue(ret_val);
  }

} // namespace other
