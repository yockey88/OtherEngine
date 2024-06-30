/**
 * \file editor\directory.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTORY_HPP
#define OTHER_ENGINE_DIRECTORY_HPP

#include <set>
#include <map>

#include "core/defines.hpp"
#include "core/ref.hpp"

#include "asset/asset_types.hpp"

namespace other {

  struct Directory : public RefCounted {
    UUID handle;
    Ref<Directory> parent_dir;

    Path path;
    std::set<AssetHandle> assets;

    std::map<UUID , Ref<Directory>> children;

    Directory() {}
    Directory(const Path& path) 
        : path(path) {}
    Directory(const Ref<Directory>& parent , const Path& path) 
      : parent_dir(parent) , path(path) {}
  };

} // namespace other

#endif // !OTHER_ENGINE_DIRECTORY_HPP
