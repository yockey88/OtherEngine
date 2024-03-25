/**
 * \file scripting/script_object.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_OBJECT_HPP
#define OTHER_ENGINE_SCRIPT_OBJECT_HPP

#include <string>

#include "core/defines.hpp"
#include "core/buffer.hpp"

namespace other {

  typedef void* ParamHandle;
  typedef ParamHandle* ParamArray;

  class Value {
    public:
      template<typename T>
      Value(const T& value) 
        : value(value) {}

    private:
      Buffer value;
  };

  class ScriptObject {
    public:
      ScriptObject(const std::string& name) 
        : script_name(name) {}
      virtual ~ScriptObject() {}

      virtual void InitializeScriptMethods() = 0;
      virtual Opt<Value> CallMethod(const std::string& name , ParamHandle* args) = 0;

      virtual void Initialize() = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void Shutdown() = 0; 
    
    protected:
      std::string script_name;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
