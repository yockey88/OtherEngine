/**
 * \file hosting/type.hpp
 **/
#ifndef DOTOTHER_TYPE_HPP
#define DOTOTHER_TYPE_HPP

#include <cstdint>
#include <utility>
#include <vector>

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"

#include "hosting/attribute.hpp"
#include "hosting/field.hpp"
#include "hosting/hosted_object.hpp"
#include "hosting/method.hpp"
#include "hosting/native_string.hpp"
#include "hosting/property.hpp"

namespace dotother {

  class Host;

  class Type {
   public:
    Type() = default;
    Type(int32_t handle)
        : handle(handle) {}
    ~Type() = default;

    void Init();

    Type& BaseObject();

    int32_t TypeSize();

    bool DerivedFrom(const Type& type);
    bool AssignableTo(const Type& type);
    bool AssignableFrom(const Type& type);

    std::vector<Field>& Fields();
    std::vector<Property>& Properties();
    std::vector<Method>& Methods();
    std::vector<Attribute>& Attributes();

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
    bool initialized = false;
    Type* base_type = nullptr;
    Type* elt_type = nullptr;

    std::vector<Field> fields;
    std::vector<Property> properties;
    std::vector<Method> methods;
    std::vector<Attribute> attributes;

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

  std::string FormatType(Type* t);

}  // namespace dotother

#endif  // !DOTOTHER_TYPE_HPP
