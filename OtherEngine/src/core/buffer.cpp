/**
 * \file core/buffer.cpp
*/
#include "core/buffer.hpp"

#include "core/logger.hpp"

namespace other {

  Buffer Buffer::Copy(const Buffer& other) {
    Buffer b;
    b.Allocate(other.size);
    memcpy(b.data , other.data , other.size);
    return b;
  }

  Buffer Buffer::Copy(const void* data , uint64_t size) {
    Buffer b;
    b.Allocate(size);
    memcpy(b.data , data , size);
    return b;
  }

  void Buffer::Allocate(uint64_t size) {
    delete[] reinterpret_cast<uint8_t*>(data);
    data = nullptr;

    if (size == 0)
      return;

    data = new uint8_t[size];
    this->size = size;
  }

  void Buffer::Release() {
    delete[] reinterpret_cast<uint8_t*>(data);
    data = nullptr;
    size = 0;
  }

  void Buffer::ZeroMem() {
    if (data != nullptr) {
      memset(data , 0 , size);
    }
  }

  uint8_t* Buffer::ReadBytes(uint64_t size , uint64_t offset) {
    OE_ASSERT(offset + size <= this->size , "Buffer::ReadBytes |> Out of bounds");
    uint8_t* ptr = new uint8_t[size];
    memcpy(ptr , (uint8_t*)data + offset , size);
    return (uint8_t*)data + offset;
  }

  void Buffer::Write(const void* data , uint64_t size , uint64_t offset) {
    OE_ASSERT(offset + size <= this->size , "Buffer::Write |> Out of bounds");
    memcpy((uint8_t*)this->data + offset , data , size);
  }

  uint8_t& Buffer::operator[](uint64_t offset) {
    return *(uint8_t*)((uint8_t*)data + offset);
  }

  uint8_t Buffer::operator[](uint64_t offset) const {
    return *(uint8_t*)((uint8_t*)data + offset);
  }

  inline uint64_t Buffer::Size() const {
    return size;
  }
  
  SafeBuffer::~SafeBuffer() {
    Release();
  }

  SafeBuffer SafeBuffer::Copy(const SafeBuffer& other) {
    SafeBuffer b;
    b.Allocate(other.Size());
    memcpy(b.data , other.data , other.Size());
    return b;
  }

} // namespace other
