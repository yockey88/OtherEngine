/**
 * \file editor\directory.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTORY_HPP
#define OTHER_ENGINE_DIRECTORY_HPP

#include <map>
#include <set>

#include "core/defines.hpp"
#include "core/directory_watcher.hpp"
#include "core/file_handle.hpp"
#include "core/ref.hpp"

#include "asset/asset_types.hpp"

namespace other {

  class Directory : public RefCounted {
   public:
    UUID handle;

    Ref<Directory> parent_dir;

    std::set<AssetHandle> assets;

    std::map<UUID, Ref<FileHandle>> file_handles;
    std::map<UUID, Ref<Directory>> children;

    Directory();
    Directory(const Path& path);
    Directory(Directory* parent, const Path& path);
    Directory(const Ref<Directory>& parent, const Path& path);

    operator Path() const;

    std::string Name() const;

    void Poll();
    bool Exists() const;
    bool Contains(const Path& path) const;
    bool Contains(UUID handle) const;

    Ref<Directory> AddFolder(const std::string_view name);
    Ref<FileHandle> AddFile(const std::string_view path);

    Ref<FileHandle> GetFile(const Path& path);
    Ref<FileHandle> GetFile(UUID handle);
    Ref<FileHandle> OpenFile(const Path& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::app);
    Ref<FileHandle> OpenFile(UUID handle, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::app);
    Ref<FileHandle> GetFileHandleByName(const std::string_view name);

    std::vector<Path> GetFilePaths(Opt<std::string> ext = std::nullopt) const;
    std::vector<Ref<FileHandle>> GetFiles(Opt<std::string> ext = std::nullopt) const;

    const Path AbsolutePath() const;
    const Path ProjectRelativePath() const;

   private:
    Ref<DirectoryWatcher> watcher = nullptr;
    Path proj_relative_path;

    void Initialize();
    void CollectChildren();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_DIRECTORY_HPP
