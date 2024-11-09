/**
 * \file core/file_handle.hpp
 **/
#ifndef OTHER_ENGINE_FILE_HANDLE_HPP
#define OTHER_ENGINE_FILE_HANDLE_HPP

#include <fstream>
#include <ios>

#include "core/defines.hpp"
#include "core/file_watcher.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "asset/asset_types.hpp"

namespace other {

  class FileHandle : public RefCounted {
   public:
    FileHandle(UUID hash, const Path& path, Opt<std::ios_base::openmode> mode = std::nullopt);
    virtual ~FileHandle();

    // void Write(T) ???

    void Open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    void Close();

    bool Exists() const;
    bool IsOpen() const;
    bool IsAsset() const;

    operator Path() const;

    AssetType GetAssetType() const;

    void Poll();

    const std::string Extension() const;
    const std::string FileName() const;
    const Path AbsolutePath() const;
    const Path ProjectRelativePath() const;

    std::string ReadString();
    std::vector<char> ReadChars();

    UUID handle = 0;

   private:
    std::fstream file;
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out;

    Path project_relative_path;
    Opt<AssetType> asset_type = std::nullopt;
    Ref<FileWatcher> watcher = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FILE_HANDLE_HPP