/**
 * \file cs_object.hpp
*/
#ifndef OTHER_ENGINE_CS_OBJECT_HPP
#define OTHER_ENGINE_CS_OBJECT_HPP

#include <map>

#include <mono/metadata/object.h>

#include "core/UUID.hpp"
#include "scripting/script_object.hpp"

namespace other {

  class CsObject : public ScriptObject {
    public:
      CsObject() 
        : ScriptObject("[Empty Script Object]") {}
      CsObject(const std::string& name , MonoClass* klass , MonoObject* instance) 
        : ScriptObject(name) , klass(klass) , instance(instance) {}
      virtual ~CsObject() override {}

      virtual void InitializeScriptMethods() override;
      virtual Opt<Value> CallMethod(const std::string& name , ParamHandle* args) override;

      virtual void Initialize() override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void Shutdown() override;

    private:
      MonoClass* klass = nullptr;
      MonoObject* instance = nullptr;

      MonoMethod* initialize_method = nullptr;
      MonoMethod* update_method = nullptr;
      MonoMethod* render_method = nullptr;
      MonoMethod* render_ui_method = nullptr;
      MonoMethod* shutdown_method = nullptr;
      std::map<UUID , MonoMethod*> other_methods;

      bool class_dirty = false;

      friend class MonoScriptModule;

      Opt<Value> CallMethod(MonoMethod* method , ParamHandle* args);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
