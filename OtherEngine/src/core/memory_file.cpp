/**
 * \file core/memory_file.cpp
 **/
#include "core/memory_file.hpp"

namespace other {

  void MemoryFile::Open(std::ios_base::openmode mode) {}

  void MemoryFile::Close() {}

  bool MemoryFile::Exists() const {
    return true;
  }

  bool MemoryFile::IsOpen() const {
    return true;
  }

  void MemoryFile::Poll() {}

  std::string MemoryFile::Extension() const {
    return ext;
  }

  std::string MemoryFile::FileName() const {
    return name;
  }

  Path MemoryFile::AbsolutePath() const {
    return Path(name + "." + ext);
  }

  Path MemoryFile::ProjectRelativePath() const {
    return Path("memory://") / (name + "." + ext);
  }

  std::istream& MemoryFile::GetReadStream() {
    OE_ASSERT(false, "MemoryFile::GetReadStream not implemented");
    throw std::runtime_error("MemoryFile::GetReadStream not implemented");
  }

  std::ostream& MemoryFile::GetWriteStream() {
    OE_ASSERT(false, "MemoryFile::GetWriteStream not implemented");
    throw std::runtime_error("MemoryFile::GetWriteStream not implemented");
  }

  std::string MemoryFile::ReadString() {
    return std::string{ &data[0], cursor };
  }

  std::vector<char> MemoryFile::ReadChars() {
    return std::vector<char>{ &data[0], &data[cursor] };
  }

}  // namespace other