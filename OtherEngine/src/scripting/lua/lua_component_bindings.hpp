/**
 * \file scripting/lua/lua_component_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP
#define OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP

#include <sol/state.hpp>

namespace other {
  namespace lua_script_bindings {

    /// small helper binding classes (since lua doesnt have u64) and to handle entity interop
    struct U64Wrapper {
      uint64_t id = 0;
      int64_t lua_id = 0;

      U64Wrapper() = default;
      U64Wrapper(uint64_t id) : id(id) {
        /// will overflow/underflow matter? since it remains unique probably not
        lua_id = static_cast<int64_t>(id);
      }

      // automatically bound for obj == obj [ __eq ]
      bool operator==(const int64_t& right) const {
        return lua_id == right;
      }
      // automatically bound for obj < obj [ __lt ]
      bool operator<(const int64_t& right) const {
        return lua_id < right;
      }
      // automatically bound for obj <= obj [ __le ]
      bool operator<=(const int64_t& right) const {
        return lua_id <= right;
      }
    };

    std::ostream& operator<<(std::ostream& os, const U64Wrapper& wrapper);

    void BindEcsTypes(sol::state& lua_state);

  }  // namespace lua_script_bindings
}  // namespace other

#endif  // !OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP
