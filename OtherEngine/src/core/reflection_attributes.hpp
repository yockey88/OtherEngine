/**
 * \file core/reflection_attributes.hpp
 **/
#ifndef OTHER_ENGINE_REFLECTION_ATTRIBUTES_HPP
#define OTHER_ENGINE_REFLECTION_ATTRIBUTES_HPP

#include <refl/refl.hpp>

namespace other {

  struct Serializable : refl::attr::usage::field,
                        refl::attr::usage::function {};

}  // namespace other

#endif  // !OTHER_ENGINE_REFLECTION_ATTRIBUTES_HPP