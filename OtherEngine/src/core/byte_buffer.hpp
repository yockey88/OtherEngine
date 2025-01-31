/**
 * \file core/byte_buffer.hpp
 **/
#ifndef OTHER_ENGINE_BYTE_BUFFER_HPP
#define OTHER_ENGINE_BYTE_BUFFER_HPP

#include <sstream>
#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "memory/buffer_allocator.hpp"

namespace other {

  class ByteBuffer {
   public:
    ByteBuffer() {}

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

    uint8_t* RawBytes() const { return data; }

    template <typename T>
    T& Read(size_t offset) {
      OE_ASSERT(offset + sizeof(T) <= bytes_written, "Attempting to read buffer as incorrectly sized type");
      return *std::launder(reinterpret_cast<T*>(data + offset));
    }

    std::string DumpBuffer() const;

   private:
    size_t bytes_written = 0;
    uint8_t* data = nullptr;

    BufferAllocator allocator;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_BYTE_BUFFER_HPP