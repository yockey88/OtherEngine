/**
 * \file scripting/lua/lua_component_bindings.cpp
 **/
#include "scripting/lua/lua_component_bindings.hpp"

#include <algorithm>

#include <entt/entt.hpp>
#include <lua/lua.h>
#include <refl/refl.hpp>
#include <reflection/object_proxy.hpp>
#include <sol/property.hpp>
#include <sol/types.hpp>

#include "asset/asset_handler.hpp"
#include "asset/asset_manager.hpp"

#include "ecs/component.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"

#include "scripting/script_engine.hpp"

namespace other {
  namespace lua_script_bindings {

    std::ostream& operator<<(std::ostream& os, const U64Wrapper& wrapper) {
      os << "[" << wrapper.id << ":" << wrapper.lua_id << "]";
      return os;
    }

    template <typename T>
      requires ComponentType<T>
    struct ComponentProxy {
      using Proxy = echo::ObjectProxy<T>;

      ComponentProxy() = default;
      ComponentProxy(ComponentType auto& component) {
        proxy = NewScope<Proxy>(&component);
      }

      sol::object GetComponentField(sol::stack_object key, sol::this_state lua);
      void SetComponentField(sol::stack_object key, sol::stack_object value, sol::this_state lua);

      Scope<Proxy> proxy = nullptr;
    };

    struct EntityProxy {
      U64Wrapper id;
      std::string name;

      using Proxy = echo::ObjectProxy<Entity>;

      EntityProxy() = default;
      EntityProxy(U64Wrapper id) {
        uint64_t uuid = id.id;
        auto scene = ScriptEngine::GetSceneContext();
        OE_ASSERT(scene != nullptr, "Scene context is null");

        Entity* entity = scene->GetEntity(UUID(uuid));
        OE_ASSERT(entity != nullptr, "Entity is null");

        proxy = NewScope<Proxy>(entity);
      }
      EntityProxy(const EntityProxy& other)
          : id(other.id), name(other.name) {
        if (other.proxy) {
          proxy = NewScope<Proxy>(other.proxy->Clone());
        }
      }
      EntityProxy(Entity* entity)
          : id(entity->GetUUID().Get()), name(entity->Name()) {
        if (entity->GetContext() != nullptr) {
          proxy = NewScope<Proxy>(entity);
        }
      }
      EntityProxy& operator=(const EntityProxy& other) {
        id = other.id;
        name = other.name;
        if (other.proxy) {
          proxy = NewScope<Proxy>(other.proxy->Clone());
        }
        return *this;
      }

      bool HasComponent(const std::string& component_name) {
        auto scene = ScriptEngine::GetSceneContext();
        OE_ASSERT(scene != nullptr, "Scene context is null");

        Entity* entity = scene->GetEntity(UUID(id.id));
        OE_ASSERT(entity != nullptr, "Entity is null");

        return entity->CheckForComponentByName(component_name);
      }

      int64_t GetId() const {
        return id.lua_id;
      }

      std::string Name() const {
        return name;
      }

      bool operator==(const EntityProxy& other) const {
        return id.id == other.id.id;
      }

      /// special component accessors
      Transform& GetTransform() {
        OE_ASSERT(proxy != nullptr, "Proxy is null");
        return proxy->target.GetComponent<Transform>();
      }

      sol::object GetComponent(sol::stack_object key, sol::this_state lua) {
        lua_State* L = lua;
        sol::stack_object selfobj(L, 1);
        EntityProxy& self = selfobj.as<EntityProxy>();
        if (self.proxy == nullptr) {
          return sol::object(lua, sol::in_place, sol::nil);
        }

        if (key.get_type() != sol::type::string) {
          return sol::object(lua, sol::in_place, sol::nil);
        }

        std::string component_name = key.as<std::string>();
        if (component_name == "name") {
          return sol::object(lua, sol::in_place, self.name);
        }
        if (component_name == "id") {
          return sol::object(lua, sol::in_place, self.id.lua_id);
        }

        {
          std::string temp_name = component_name;
          std::ranges::replace(temp_name, '_', '-');
          if (!self.HasComponent(temp_name)) {
            return sol::object(lua, sol::in_place, sol::nil);
          }
        }

        using namespace std::string_view_literals;
        uint64_t hash = FNV(component_name);
        switch (hash) {
          case FNV("mesh"sv): {
            Mesh& m = self.proxy->target.GetComponent<Mesh>();
            return sol::object(lua, sol::in_place, ComponentProxy<Mesh>(m));
          }
          case FNV("static_mesh"sv): {
            StaticMesh& sm = self.proxy->target.GetComponent<StaticMesh>();
            return sol::object(lua, sol::in_place, ComponentProxy<StaticMesh>(sm));
          }
          default: {
            OE_ERROR("Component {} not found", component_name);
            return sol::object(lua, sol::in_place, sol::nil);
          }
        }
      }

      Scope<Proxy> proxy = nullptr;
    };

    void BindTransform(sol::state& lua_state);
    void BindMesh(sol::state& lua_state);

    void BindEcsTypes(sol::state& lua_state) {
      using namespace entt::literals;

      lua_state.new_usertype<U64Wrapper>(
        "U64",
        sol::meta_function::construct,
        sol::factories(
          []() { return U64Wrapper(); },
          [](int64_t id) { return U64Wrapper(static_cast<uint64_t>(id)); }
        )
      );

      BindTransform(lua_state);
      BindMesh(lua_state);

      lua_state.new_usertype<EntityProxy>(
        "Entity",
        sol::constructors<EntityProxy(U64Wrapper)>(),
        sol::meta_function::index, &EntityProxy::GetComponent,
        sol::meta_function::equal_to, &EntityProxy::operator==,
        "HasComponent", &EntityProxy::HasComponent,
        "transform", sol::property(&EntityProxy::GetTransform)
      );

      lua_state["Scene"] = lua_state.create_table_with(
        "ContextHandle",
        []() -> int64_t {
          auto scene = ScriptEngine::GetSceneContext();
          if (scene == nullptr) {
            return 0;
          }

          U64Wrapper handle(scene->handle.Get());
          return handle.lua_id;
        },
        "EntityIds",
        []() -> std::vector<int64_t> {
          auto scene = ScriptEngine::GetSceneContext();
          if (scene == nullptr) {
            return {};
          }

          std::vector<int64_t> entities;
          for (const auto& [id, entity] : scene->SceneEntities()) {
            entities.push_back(U64Wrapper(id.Get()).lua_id);
          }
          return entities;
        },
        "SceneEntities",
        []() -> std::vector<EntityProxy> {
          auto scene = ScriptEngine::GetSceneContext();
          if (scene == nullptr) {
            return {};
          }

          std::vector<EntityProxy> entities;
          for (const auto& [id, entity] : scene->SceneEntities()) {
            entities.emplace_back(entity);
          }
          return entities;
        }
      );
    }

    void BindTransform(sol::state& lua_state) {
      lua_state.new_usertype<Transform>(
        "Transform",
        "position", &Transform::position,
        "rotation", &Transform::erotation,
        "scale", &Transform::scale
      );
    }

    template <>
    sol::object ComponentProxy<Mesh>::GetComponentField(sol::stack_object key, sol::this_state lua) {
      lua_State* L = lua;
      sol::stack_object selfobj(L, 1);
      ComponentProxy<Mesh>& self = selfobj.as<ComponentProxy<Mesh>>();

      if (key.get_type() != sol::type::string) {
        return sol::object(lua, sol::in_place, sol::nil);
      }

      std::string field_name = key.as<std::string>();

      if (field_name == "mesh") {
        U64Wrapper id{ self.proxy->target.handle.id.Get() };
        return sol::object(lua, sol::in_place, id.lua_id);
      }

      return sol::object(lua, sol::in_place, sol::nil);
    }

    template <>
    sol::object ComponentProxy<StaticMesh>::GetComponentField(sol::stack_object key, sol::this_state lua) {
      lua_State* L = lua;
      sol::stack_object selfobj(L, 1);
      ComponentProxy<StaticMesh>& self = selfobj.as<ComponentProxy<StaticMesh>>();

      if (key.get_type() != sol::type::string) {
        return sol::object(lua, sol::in_place, sol::nil);
      }

      std::string field_name = key.as<std::string>();

      if (field_name == "mesh") {
        U64Wrapper id{ self.proxy->target.handle.id.Get() };
        return sol::object(lua, sol::in_place, id.lua_id);
      }

      return sol::object(lua, sol::in_place, sol::nil);
    }

    template <>
    void ComponentProxy<Mesh>::SetComponentField(sol::stack_object key, sol::stack_object value, sol::this_state lua) {
      lua_State* L = lua;
      sol::stack_object selfobj(L, 1);
      ComponentProxy<Mesh>& self = selfobj.as<ComponentProxy<Mesh>>();

      if (key.get_type() != sol::type::string) {
        return;
      }

      std::string field_name = key.as<std::string>();

      if (field_name == "mesh") {
        if (value.get_type() != sol::type::number) {
          return;
        }

        int64_t id = value.as<int64_t>();
        AssetHandle temp_handle = AssetHandle{ U64Wrapper(id).id };
        bool handle_valid = AppState::Assets()->IsHandleValid(temp_handle);
        if (!handle_valid) {
          OE_ERROR("Invalid asset handle {}", id);
          return;
        }

        self.proxy->target.handle = temp_handle;
      }
    }

    template <>
    void ComponentProxy<StaticMesh>::SetComponentField(sol::stack_object key, sol::stack_object value, sol::this_state lua) {
      lua_State* L = lua;
      sol::stack_object selfobj(L, 1);
      ComponentProxy<StaticMesh>& self = selfobj.as<ComponentProxy<StaticMesh>>();

      if (key.get_type() != sol::type::string) {
        return;
      }

      std::string field_name = key.as<std::string>();

      if (field_name == "mesh") {
        if (value.get_type() != sol::type::number) {
          return;
        }

        int64_t id = value.as<int64_t>();
        AssetHandle temp_handle = AssetHandle{ U64Wrapper(id).id };
        bool handle_valid = AppState::Assets()->IsHandleValid(temp_handle);
        if (!handle_valid) {
          OE_ERROR("Invalid asset handle {}", id);
          return;
        }

        self.proxy->target.handle = temp_handle;
      }
    }

    void BindMesh(sol::state& lua_state) {
      lua_state.new_usertype<Material>(
        "Material",
        sol::constructors<
          Material(),
          Material(const glm::vec4&, float)>(),
        "color", &Material::color,
        "shininess", &Material::shininess
      );

      // clang-format off
      lua_state.new_usertype<ComponentProxy<Mesh>>(
        "Mesh",
        sol::meta_function::index,
        &ComponentProxy<Mesh>::GetComponentField,
        sol::meta_function::new_index,
        &ComponentProxy<Mesh>::SetComponentField,
        "material", sol::property(
          [](ComponentProxy<Mesh>& self) -> Material& {
            return self.proxy->target.material;
          },
          [](ComponentProxy<Mesh>& self, const Material& mat) {
            self.proxy->target.material = mat;
          }
        )
      );

      lua_state.new_usertype<ComponentProxy<StaticMesh>>(
        "StaticMesh",
        sol::meta_function::index,
        &ComponentProxy<StaticMesh>::GetComponentField,
        sol::meta_function::new_index,
        &ComponentProxy<StaticMesh>::SetComponentField,
        "material", sol::property(
          [](ComponentProxy<StaticMesh>& self) -> Material& {
            return self.proxy->target.material;
          },
          [](ComponentProxy<StaticMesh>& self, const Material& mat) {
            self.proxy->target.material = mat;
          }
        )
      );
      // clang-format on
    }

  }  // namespace lua_script_bindings
}  // namespace other
