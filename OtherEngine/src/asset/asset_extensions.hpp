/**
 * \file asset/asset_extensions.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_EXTENSIONS_HPP
#define OTHER_ENGINE_ASSET_EXTENSIONS_HPP

#include <map>

#include "core/uuid.hpp"
#include "asset/asset_types.hpp"

namespace other {

  static const std::map<UUID , AssetType> asset_extensions = {
    /// scene extensions
    { FNV(".yscn") , AssetType::SCENE } ,
    { FNV(".yscene") , AssetType::SCENE } ,

    /// prefabs

    /// mesh/animations
    { FNV(".fbx") , AssetType::MODEL } ,
    { FNV(".gltf") , AssetType::MODEL } ,
    { FNV(".glb") , AssetType::MODEL } ,
    { FNV(".obj") , AssetType::MODEL } ,

    /// shaders
    { FNV(".glsl")    , AssetType::SHADER } ,
    { FNV(".vert")    , AssetType::SHADER } ,
    { FNV(".frag")    , AssetType::SHADER } ,
    { FNV(".geom")    , AssetType::SHADER } ,
    { FNV(".oshader") , AssetType::SHADER } ,

    /// materials

    /// textures
    { FNV(".png") , AssetType::TEXTURE } ,
    { FNV(".jpg") , AssetType::TEXTURE } ,
    { FNV(".jpeg") , AssetType::TEXTURE } ,

    /// env maps 
    /// audio
    /// fonts
    /// { FNV(".ttf") , AssetType::FONT } ,
    /// { FNV(".ttc") , AssetType::FONT } ,
    /// { FNV(".otf") , AssetType::FONT } ,

    /// scripts
    { FNV(".cs") , AssetType::SCRIPTFILE } , 
    { FNV(".lua") , AssetType::SCRIPTFILE } ,

    // source 
    { FNV(".h") , AssetType::SOURCE_FILE } ,
    { FNV(".hpp") , AssetType::SOURCE_FILE } ,
    { FNV(".cpp") , AssetType::SOURCE_FILE } ,
  };

} // nmamespace other

#endif // !OTHER_ENGINE_ASSET_EXTENSIONS_HPP
