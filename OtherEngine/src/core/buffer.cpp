/**
 * \file core/buffer.cpp
*/
#include "core/buffer.hpp"

#include "core/logger.hpp"

namespace other {
      
  Buffer::Buffer(void* data , uint64_t size) {
    Allocate(size);
    Write(data , size);
  }

  Buffer::Buffer(Buffer&& other) {
    Allocate(other.size);
    Write(other.data , other.size);
    other.Release();
  }

  Buffer::Buffer(const Buffer& other) {
    Allocate(other.size);
    Write(other.data , other.size);
  }

  Buffer& Buffer::operator=(Buffer&& other) {
    Allocate(other.size);
    Write(other.data , other.size);
    other.Release();
    return *this;
  }

  Buffer& Buffer::operator=(const Buffer& other) {
    Allocate(other.size);
    Write(other.data , other.size);
    return *this;
  }

  Buffer Buffer::Copy(const Buffer& other) {
    Buffer b;
    b.Allocate(other.size);
    b.Write(other.data , other.size);
    return b;
  }

  Buffer Buffer::Copy(const void* data , uint64_t size) {
    Buffer b;
    b.Allocate(size);
    b.Write(data , size);
    return b;
  }

  void Buffer::Allocate(uint64_t sz) {
    delete[] data;
    data = nullptr;

    if (sz == 0) {
      return;
    }

    data = new uint8_t[sz];
    size = sz;
  }

  void Buffer::Release() {
    delete[] data;
    data = nullptr;
    size = 0;
  }

  void Buffer::ZeroMem() {
    if (data != nullptr) {
      memset(data , 0 , size);
    }
  }

  const uint8_t* Buffer::ReadBytes(uint64_t offset) const {
    OE_ASSERT(offset < size , "Buffer::ReadBytes |> Out of bounds");
    return static_cast<const uint8_t*>(&data[offset]);
  }

  void Buffer::Write(const void* d , uint64_t sz , uint64_t offset) {
    OE_ASSERT(sz <= size , "Buffer::Write |> Out of bounds ({} vs {})" , sz , size);
    memcpy(data , d , size);
    size = sz;
  }

  uint8_t& Buffer::operator[](uint64_t offset) {
    OE_ASSERT(offset >= size , "Accessing Buffer out of bounds");
    return *(data + offset);
  }

  uint8_t Buffer::operator[](uint64_t offset) const {
    OE_ASSERT(offset >= size , "Accessing Buffer out of bounds");
    return *(data + offset);
  }

  uint64_t Buffer::Size() const {
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
