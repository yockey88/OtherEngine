/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <string>
#include <map>

#include "core/value.hpp"
#include "scripting/script_field.hpp"

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
      bool IsInitialized() const;

      const std::map<UUID , ScriptField>& GetFields();

      virtual void InitializeScriptMethods() = 0;
      virtual void InitializeScriptFields() = 0;
      virtual Opt<Value> CallMethod(const std::string& name , Parameter* args , uint32_t argc) = 0;

      virtual void Initialize() = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void Shutdown() = 0; 
    
    protected:
      bool is_initialized = false;
      bool is_corrupt = false;
      std::string script_name;
      std::string language;

      std::map<UUID , ScriptField> fields;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
