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
#include "core/view.hpp"

#include "asset/asset_defines.hpp"

namespace other {

  struct CreateFileEvent;

  class Directory : public RefCounted {
   public:
    UUID handle;

    View<Directory> parent_dir;

    std::map<UUID, Ref<FileHandle>> file_handles;
    std::map<UUID, Ref<Directory>> children;

    Directory();
    Directory(const Path& path, UUID hash);
    Directory(Directory* parent, const Path& path, UUID hash);
    Directory(Ref<Directory>& parent, const Path& path, UUID hash);

    operator Path() const;

    static std::vector<std::string> SplitPath(const std::string_view path);

    std::string Name() const;

    void Poll();
    void Update();
    bool Exists() const;
    bool Contains(const Path& path) const;
    bool Contains(UUID handle) const;

    Ref<Directory> AddFolder(const std::string_view name);
    Ref<FileHandle> AddFile(const std::string_view path);
    bool RemoveFile(UUID handle);
    bool RemoveChildDirectory(UUID handle);

    Ref<Directory> GetChildDirectory(const std::string_view name);

    Ref<FileHandle> GetFile(const Path& path);
    Ref<FileHandle> GetFile(UUID handle);
    Ref<FileHandle> OpenFile(const Path& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::app);
    Ref<FileHandle> OpenFile(UUID handle, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::app);
    Ref<FileHandle> GetFileHandleByName(const std::string_view name, Opt<std::string> ext = std::nullopt);

    std::vector<Path> GetFilePaths(Opt<std::string> ext = std::nullopt) const;
    std::vector<Ref<FileHandle>> GetFiles(Opt<std::string> ext = std::nullopt) const;

    const Path AbsolutePath() const;
    const Path ProjectRelativePath() const;

   private:
    Ref<DirectoryWatcher> watcher = nullptr;
    Path proj_relative_path;

    void Initialize(bool create_dir_handles);
    void CollectChildren(bool create_dir_handles);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_DIRECTORY_HPP
