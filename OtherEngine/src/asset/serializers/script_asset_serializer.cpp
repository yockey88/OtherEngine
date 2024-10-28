/**
 * \file asset/serializers/script_asset_serializer.cpp
 **/
#include "asset/serializers/script_asset_serializer.hpp"

#include "core/logger.hpp"
#include "core/ref.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/script_module.hpp"

namespace other {

  void ScriptFileAssetSerializer::Serialize(const Ref<Asset>& asset) {
    OE_ASSERT(false, "ScriptFileAssetSerializer::Serialize unimplemented");
  }

  void ScriptFileAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) {
    OE_ASSERT(false, "ScriptFileAssetSerializer::Serialize unimplemented");
  }

  bool ScriptFileAssetSerializer::Load(const AssetMetadata& metadata, Ref<Asset>& asset) {
    OE_ASSERT(metadata.type == AssetType::SCRIPTFILE, "Asset type mismatch");
    Ref<ScriptModule> script_module = ScriptEngine::GetScriptModule(metadata.handle.id);
    if (script_module == nullptr) {
      script_module = ScriptEngine::GetScriptModule(metadata.path.string());
    }

    auto ext = metadata.path.extension().string();

    return false;
  }

}  // namespace other