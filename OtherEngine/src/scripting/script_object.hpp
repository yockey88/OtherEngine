/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <string>

#include "core/defines.hpp"
#include "core/value.hpp"

namespace other {

  class ScriptObject {
    public:
      ScriptObject(const std::string& name , const std::string& language) 
        : script_name(name) , language(language) {}
      virtual ~ScriptObject() {}

      const std::string& Name() const;
      const std::string& LanguageName() const;

      void MarkCorrupt();
      bool IsCorrupt() const;

      virtual void InitializeScriptMethods() = 0;
      virtual void CallMethod(Value* ret , const std::string& name , ParamHandle* args) = 0;
      virtual void GetProperty(Value* ret , const std::string& name) const = 0;
      virtual void SetProperty(const Value& value) = 0;

      virtual void Initialize() = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void Shutdown() = 0; 
    
    protected:
      bool is_corrupt = false;
      std::string script_name;
      std::string language;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
