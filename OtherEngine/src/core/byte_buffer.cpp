/**
 * \file core/byte_buffer.cpp
 **/
#include "core/byte_buffer.hpp"

namespace other {

  void ByteBuffer::BufferBytes(const void* new_data, size_t size) {
    size_t new_size = bytes_written + size;
    uint8_t* new_buff = allocator.Allocate(new_size);

    if (data != nullptr) {
      std::memcpy(new_buff, data, bytes_written);
      allocator.Free(data, bytes_written);
    }

    std::memcpy(new_buff + bytes_written, new_data, size);
    data = new_buff;
    bytes_written += size;
  }

  void ByteBuffer::Write(const void* new_data, size_t size) {
    Release();

    data = allocator.Allocate(size);

    std::memcpy(data, new_data, size);
    bytes_written += size;
  }

  void ByteBuffer::Release() {
    if (data != nullptr) {
      allocator.Free(data, bytes_written);
      data = nullptr;
      bytes_written = 0;
    }
  }

  size_t ByteBuffer::Size() const {
    return bytes_written;
  }

  bool ByteBuffer::Empty() const {
    return data == nullptr || bytes_written == 0;
  }

  std::string ByteBuffer::DumpBuffer() const {
    std::stringstream ss;

    if (data == nullptr) {
      ss << "[ EMPTY ]";
      return ss.str();
    }
    ss << "Bytes written = " << bytes_written;

    for (uint32_t i = 0; i < bytes_written; ++i) {
      using namespace std::string_view_literals;
      if (i % 16 == 0) {
        ss << "\n"
           << fmt::format("{:0>04d} : "sv, i);
      }

      ss << fmt::format("{:#04x} "sv, data[i]);
    }
    ss << "\n";

    return ss.str();
  }

}  // namespace other