/**
 * \file core/type_data.hpp
 **/
#ifndef OTHER_ENGINE_TYPE_DATA_HPP
#define OTHER_ENGINE_TYPE_DATA_HPP

#include <string_view>

#include "core/defines.hpp"

namespace other {

  template<typename T , bool NS>
  class TypeDataBase {
    protected:
      std::string_view DemangleName(std::string_view type_name) const {
        size_t space = type_name.find(' ');
        if (space != std::string_view::npos) {
          type_name.remove_prefix(space + 1);
        }

        if constexpr (NS) {
          size_t namespace_name = type_name.find("::");
          if (namespace_name != std::string_view::npos) {
              type_name.remove_prefix(namespace_name + 2);
          }
        }

        return type_name;
      }
  };

  template<typename T , bool NS = false>
  class TypeData : public TypeDataBase<T , NS> {
    public:
      constexpr TypeData()
        : name(TypeDataBase<T , NS>::DemangleName(typeid(T).name())) {}

      std::string_view Name() { return name; }
      const std::string_view& Name() const { return name; }
      uint64_t Hash() const { return FNV(name); }

    private:
      std::string_view name; 
  };


} // namespace other

#endif // !OTHER_ENGINE_TYPE_DATA_HPP
