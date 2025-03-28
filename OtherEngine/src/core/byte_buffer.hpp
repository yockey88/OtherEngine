/**
 * \file core/byte_buffer.hpp
 **/
#ifndef OTHER_ENGINE_BYTE_BUFFER_HPP
#define OTHER_ENGINE_BYTE_BUFFER_HPP

#include <spdlog/fmt/fmt.h>

#include "core/logger.hpp"
#include "memory/buffer_allocator.hpp"

namespace other {

  class ByteBuffer {
   public:
    ByteBuffer() {}

    /// FIXME: don't resize any time even if arena makes it fast to do so
    template <typename T>
    void BufferData(const T& value) {
      size_t new_size = bytes_written + sizeof(T);
      uint8_t* new_buff = allocator.Allocate(new_size);

      if (data != nullptr) {
        std::memcpy(new_buff, data, bytes_written);
        allocator.Free(data, bytes_written);
      }

      std::memcpy(new_buff + bytes_written, (void*)&value, sizeof(T));
      data = new_buff;
      bytes_written += sizeof(T);
    }

    template <>
    void BufferData<std::string>(const std::string& value) {
      size_t new_size = bytes_written + (value.size() + 1);  /// include null terminator
      uint8_t* new_buff = allocator.Allocate(new_size);

      if (data != nullptr) {
        std::memcpy(new_buff, data, bytes_written);
        allocator.Free(data, bytes_written);
      }

      std::memcpy(new_buff + bytes_written, value.c_str(), value.size());
      data = new_buff;
      bytes_written += value.size();

      /// null terminator
      data[bytes_written] = '\0';
      bytes_written++;
    }

    void BufferBytes(const void* data, size_t size);
    void Write(const void* data, size_t size);

    uint8_t* RawBytes() const { return data; }

    template <typename T>
    T& Read(size_t offset) {
      OE_ASSERT(offset + sizeof(T) <= bytes_written, "Attempting to read buffer as incorrectly sized type [T = {}] : {} + {} > {}", typeid(T).name(), offset, sizeof(T), bytes_written);
      return *std::launder(reinterpret_cast<T*>(data + offset));
    }

    std::string ReadStr(size_t offset) {
      OE_ASSERT(offset < bytes_written, "Attempting to read string from buffer at invalid offset : {} > {}", offset, bytes_written);

      size_t len = 0;
      for (size_t i = offset; i < bytes_written; ++i) {
        if (i == bytes_written - 1 && data[i] != '\0') {
          len = i - offset;
          break;
        }

        if (data[i] == '\0') {
          len = i - offset;
          break;
        }
      }
      if (len == 0) {
        return "";
      }
      OE_ASSERT(offset + len <= bytes_written, "Attempting to read string from buffer at invalid offset : {} + {} > {}", offset, len, bytes_written);

      const char* str = std::launder(reinterpret_cast<const char*>(data + offset));
      return std::string(str, len);
    }

    void Release();
    size_t Size() const;
    bool Empty() const;

    std::string DumpBuffer() const;

   private:
    size_t bytes_written = 0;
    uint8_t* data = nullptr;

    BufferAllocator allocator;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_BYTE_BUFFER_HPP