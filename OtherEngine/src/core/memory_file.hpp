/**
 * \file core/memory_file.hpp
 **/
#ifndef OTHER_ENGINE_MEMORY_FILE_HPP
#define OTHER_ENGINE_MEMORY_FILE_HPP

#include <string_view>

#include "core/file_handle.hpp"

namespace other {

  class MemoryFile : public FileHandle {
   public:
    MemoryFile(const std::string_view name, const std::string_view ext)
        : name(name), ext(ext) {}
    virtual ~MemoryFile() override = default;

    virtual void Open(std::ios_base::openmode mode = kDefaultOpenMode) override;
    virtual void Close() override;

    virtual bool Exists() const override;
    virtual bool IsOpen() const override;

    virtual void Poll() override;

    virtual std::string Extension() const override;
    virtual std::string FileName() const override;
    virtual Path AbsolutePath() const override;
    virtual Path ProjectRelativePath() const override;

    virtual std::istream& GetReadStream() override;
    virtual std::ostream& GetWriteStream() override;

    virtual std::string ReadString() override;
    virtual std::vector<char> ReadChars() override;

   public:
    std::string name;
    std::string ext;

    // constexpr static size_t kNumMemPages = 16;
    constexpr static size_t kMemPageSize = 4096;

    uint16_t cursor = 0;
    char data[kMemPageSize] = { 0 };
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MEMORY_FILE_HPP