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

#include "asset/asset_defines.hpp"

#include "writer_reader.hpp"

namespace other {

  class FileHandle : public RefCounted {
   public:
    constexpr static std::ios_base::openmode kDefaultOpenMode = std::ios_base::in | std::ios_base::out | std::ios_base::app;

    FileHandle(const Path& path, Opt<std::ios_base::openmode> mode = std::nullopt);
    virtual ~FileHandle() override;

    virtual void Open(std::ios_base::openmode mode = kDefaultOpenMode);
    virtual void Close();

    virtual bool Exists() const;
    virtual bool IsOpen() const;
    virtual bool Remove();

    operator Path() const;

    AssetType GetAssetType() const;

    virtual void Poll();

    virtual std::string Extension() const;
    virtual std::string FileName() const;
    virtual Path AbsolutePath() const;
    virtual Path ProjectRelativePath() const;

    virtual std::istream& GetReadStream();
    virtual std::ostream& GetWriteStream();

    template <typename T>
      requires Writable<T>
    void Write(const T& data) {
      if (!IsOpen() || !(mode & std::ios_base::out)) {
        OE_ERROR("File {} is not open for writing", project_relative_path.string());
        return;
      }

      Writer<T>{}(GetWriteStream(), data);
    }

    void WriteJson(const nlohmann::json& data);

    template <typename T>
      requires Readable<T>
    T Read() {
      return Reader<T>{}(GetReadStream());
    }

    virtual std::string ReadString();
    virtual std::vector<char> ReadChars();
    virtual std::vector<uint8_t> ReadBytes();

    UUID handle = 0;

   protected:
    FileHandle() = default;

   private:
    std::fstream file;
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out;

    Path project_relative_path;
    AssetType asset_type = AssetType::GENERIC_FILE;
    Ref<FileWatcher> watcher = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FILE_HANDLE_HPP