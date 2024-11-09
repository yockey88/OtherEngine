/**
 * \file core/type_data.hpp
 **/
#ifndef OTHER_ENGINE_TYPE_DATA_HPP
#define OTHER_ENGINE_TYPE_DATA_HPP

#include <string_view>

#include <refl/refl.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/reflection_attributes.hpp"

namespace other {

  template <typename T>
  class TypeData {
   public:
    constexpr TypeData()
        : desc(refl::reflect<T>()), raw_name(typeid(T).name()),
          name(DemangleName(typeid(T).name())) {}

    std::string_view Name() {
      return name;
    }

    const std::string_view& Name() const {
      return name;
    }

    uint64_t Hash() const {
      return FNV(name);
    }

    // void Serialize(std::ostream& stream, const T& obj) {
    //   stream << "<" << name << ">" << " = {\n";
    //   refl::util::for_each(desc.members, [&](auto member) {
    //     if constexpr (refl::descriptor::is_readable(member) && refl::descriptor::has_attribute<Serializable>(member)) {
    //       stream << "  " << refl::descriptor::get_display_name(member) << " = ";
    //       auto value = member(obj);
    //       if constexpr (requires { stream << value; }) {
    //         stream << value;
    //       } else {
    //         OE_ERROR("Failed to serialize member {}", refl::descriptor::get_display_name(member));
    //       }
    //       stream << "\n";
    //     }
    //   });
    //   stream << "}\n";
    // }

   private:
    refl::type_descriptor<T> desc;
    std::string_view raw_name;
    std::string_view name;

    std::string_view

    DemangleName(std::string_view type_name) const {
      size_t space = type_name.find(' ');
      if (space != std::string_view::npos) {
        type_name.remove_prefix(space + 1);
      }

      size_t namespace_name = type_name.find("::");
      if (namespace_name != std::string_view::npos) {
        type_name.remove_prefix(namespace_name + 2);
      }

      return type_name;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TYPE_DATA_HPP
