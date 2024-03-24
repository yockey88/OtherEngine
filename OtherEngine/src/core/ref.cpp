/**
 * \file core/ref.hpp
*/
#include "core/ref.hpp"

#include <unordered_set>

namespace other {
namespace {

  std::unordered_set<void*> refs;

} // namespace anon
namespace detail {

  void RegisterReference(void* instance) {
    refs.insert(instance);
  }

  void RemoveReference(void* instance) {
    refs.erase(instance);
  }

  bool IsValidRef(void* instance) {
    return refs.find(instance) != refs.end();
  }

} // namespace detail
} // namespace other
