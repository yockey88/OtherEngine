/**
 * \file asset/serializers/shader_serializer.cpp
 **/
#include "asset/serializers/shader_serializer.hpp"

#include "core/filesystem.hpp"

#include "asset/asset_database.hpp"
#include "parsing/shader_compiler.hpp"

namespace other {

  void ShaderSerializer::Serialize(const AssetMetadata& metadata) {
  }

  bool ShaderSerializer::Load(AssetMetadata& metadata) {
    OE_INFO("Attempting to build shader : {}", metadata.path);
    std::string src = "";
    AssetKey key;
    {
      Ref<FileHandle> shaderfile = Filesystem::GetFile(metadata.path);
      if (shaderfile == nullptr) {
        OE_ERROR("Failed to get shader file : {}", metadata.path);
        return false;
      }

      shaderfile->Open(std::ios::in);
      src = shaderfile->ReadString();
      if (src.empty()) {
        OE_ERROR("Failed to read shader file {}", metadata.path);
        return false;
      }
      shaderfile->Close();

      key = {
        .file_handle = shaderfile->handle,
        .type = shaderfile->GetAssetType(),
      };
    }

    ShaderIr ir = ShaderCompiler::Compile(src);
    ir.name = metadata.path.filename().string();

    OE_INFO(" > Built shader : {}", ir.name);
    metadata.asset = NewRef<Shader>(ir);
    return true;
  }

}  // namespace other