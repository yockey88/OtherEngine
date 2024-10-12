/**
 * \file hosting/type.hpp
 **/
#ifndef DOTOTHER_TYPE_HPP
#define DOTOTHER_TYPE_HPP

#include <cstdint>
#include <utility>
#include <vector>

#include "core/dotother_defines.hpp"
#include "hosting/native_string.hpp"
#include "hosting/hosted_object.hpp"
#include "core/utilities.hpp"

namespace dotother {

  class Host;

  class Field;
  class Property;
  class Attribute;
  class Method;

  class Type {
    public:
      Type() 
        : handle(-1) {}
      Type(int32_t handle)
          : handle(handle) {}
      ~Type() = default;

      Type& BaseObject();

      int32_t TypeSize();

      bool DerivedFrom(const Type& type);
      bool AssignableTo(const Type& type);
      bool AssignableFrom(const Type& type);

      std::vector<Field> Fields();
      std::vector<Property> Properties();
      std::vector<Method> Methods();
      std::vector<Attribute> Attributes();

      bool HasAttribute(const Type& type);

      ManagedType GetManagedType();

      bool IsArray();
      Type& GetEltType();

      bool operator==(const Type& other);
      operator bool();

      NString FullName();

      template <typename... Args>
      HostedObject NewInstance(Args&&... args) {
        constexpr size_t argc = sizeof...(args);

        HostedObject res;
        if constexpr (argc > 0) {
          const void* argv[argc] = {};
          ManagedType arg_ts[argc] = {};
          util::AddToArray<Args...>(argv, arg_ts, std::forward<Args>(args)..., std::make_index_sequence<argc>{});
          res = New(argv, arg_ts, argc);
        } else {
          res = New(nullptr, nullptr, 0);
        }

        return res;
      }

      HostedObject New(const void** argv, const ManagedType* arg_ts, size_t argc);

      int32_t handle = -1;

    private:
      Type* base_type = nullptr;
      Type* elt_type = nullptr;

      void CheckHost();
      void LoadTag();

      friend class Host;
      friend class AssemblyContext;
      friend class Assembly;

      friend class Field;
      friend class Property;
      friend class Attribute;
      friend class Method;
  };

} // namespace dotother

#endif // !DOTOTHER_TYPE_HPP
