/**
 * \file hosting/method.hpp
 **/
#ifndef DOTOTHER_METHOD_HPP
#define DOTOTHER_METHOD_HPP

#include <vector>

#include "hosting/native_string.hpp"

namespace dotother {

  class Type;
  class Attribute; 

  class Method {
    public:
      Method() {}

      NString GetName() const;
      Type& GetReturnType();
      const std::vector<Type*>& ParamTypes();

      TypeAccessibility Accessibility() const;
      std::vector<Attribute> Attributes() const;

      int32_t handle = -1;
      
    private:
      Type* ret_type = nullptr;
      std::vector<Type*> param_types{};

      friend class Type;
  };

} // namespace dotother

#endif // !DOTOTHER_METHOD_HPP
