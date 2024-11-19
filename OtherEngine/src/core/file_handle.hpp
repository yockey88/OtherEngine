/**
 * \file core/file_handle.hpp
 **/
#ifndef OTHER_ENGINE_FILE_HANDLE_HPP
#define OTHER_ENGINE_FILE_HANDLE_HPP

#include <fstream>
#include <ios>
#include <sstream>

#include "core/file_watcher.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"
#include "core/writer_reader.hpp"

#include "asset/asset_types.hpp"

#include "writer_reader.hpp"

namespace other {

  class FileHandle : public RefCounted {
   public:
    constexpr static std::ios_base::openmode kDefaultOpenMode = std::ios_base::in | std::ios_base::out | std::ios_base::app;

    FileHandle(UUID hash, const Path& path, Opt<std::ios_base::openmode> mode = std::nullopt);
    virtual ~FileHandle();

    void Open(std::ios_base::openmode mode = kDefaultOpenMode);
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

    std::fstream& RawFileStream();

    template <typename T>
      requires Writable<T>
    void Write(const T& data) {
      if (!IsOpen() || !(mode & std::ios_base::out)) {
        OE_ERROR("File {} is not open for writing", project_relative_path.string());
        return;
      }

      Writer<T>{}(file, data);
    }

    void WriteJson(const nlohmann::json& data);

    template <typename T>
      requires Readable<T>
    T Read() {
      return Reader<T>{}(file);
    }

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