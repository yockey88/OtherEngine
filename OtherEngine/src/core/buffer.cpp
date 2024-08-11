/**
 * \file core/buffer.cpp
*/
#include "core/buffer.hpp"

#include <cstring>
#include <numeric>

#include "core/logger.hpp"

namespace other {
      
  Buffer::Buffer(void* d , uint64_t sz) {
    *this = Copy(d , sz);
  }

  Buffer::Buffer(Buffer&& other) {
    *this = Copy(other);
    other.Release();
  }

  Buffer::Buffer(const Buffer& other) {
    *this = Copy(other);
  }

  Buffer& Buffer::operator=(Buffer&& other) {
    Allocate(other.capacity);
    Write(other.data , other.capacity);
    other.Release();
    return *this;
  }

  Buffer& Buffer::operator=(const Buffer& other) {
    *this = Copy(other);
    return *this;
  }

  Buffer Buffer::Copy(const Buffer& other) {
    Buffer b;
    b.Allocate(other.capacity);
    b.Write(other.data , other.capacity);
    return b;
  }

  Buffer Buffer::Copy(const void* d , uint64_t sz) {
    Buffer b;
    b.Allocate(sz);
    b.Write(d , sz);
    return b;
  }

  void Buffer::Allocate(uint64_t sz) {
    Release();

    if (sz == 0) {
      return;
    }

    capacity = sz;
    data = new uint8_t[capacity];
    ZeroMem();
  }
      
  void Buffer::Extend() {
    size_t new_size = 2 * capacity;
    uint8_t* new_buffer = new uint8_t[new_size];
    uint8_t* temp = data;

    memset(new_buffer , 0 , new_size);
    memcpy(new_buffer , data , capacity);

    data = new_buffer;
    delete[] temp;

    capacity = new_size;
  }

  void Buffer::Release() {
    delete[] data;
    data = nullptr;
    capacity = 0;
    element_sizes.clear();
  }

  void Buffer::ZeroMem() {
    if (data != nullptr) {
      memset(data , 0 , capacity);
    }
  }
      
  size_t Buffer::ElementSize(size_t index) const {
    if (index >= element_sizes.size()) {
      return 0;
    }

    return element_sizes[index];
  }

  const uint8_t* Buffer::ReadBytes(uint64_t offset) const {
    OE_ASSERT(offset <= capacity , "Buffer::ReadBytes |> Out of bounds! attempted read at {} > {} real capacity" , offset , capacity);
    return static_cast<const uint8_t*>(&data[offset]);
  }

  void Buffer::Write(const void* d , uint64_t sz , uint64_t offset) {
    OE_ASSERT(offset + sz <= capacity , "Attempting to write into invalid memory! expected capacity {} + {} = {} > {} real capacity" , 
              offset , sz , offset + sz , capacity);
    const uint8_t* d_bytes = reinterpret_cast<const uint8_t*>(d);
    uint8_t* data_bytes = reinterpret_cast<uint8_t*>(data);

    for (size_t i = offset; i < offset + sz; ++i) {
      data_bytes[i] = d_bytes[i];
    }
  }

  uint8_t& Buffer::operator[](uint64_t offset) {
    OE_ASSERT(offset <= capacity , "Accessing Buffer out of bounds");
    return *(data + offset);
  }

  uint8_t Buffer::operator[](uint64_t offset) const {
    OE_ASSERT(offset <= capacity , "Accessing Buffer out of bounds");
    return *(data + offset);
  }
      
  std::string Buffer::DumpBuffer() const {
    std::stringstream ss;

    ss << "Buffer Capacity = " << Capacity() << "\n";
    ss << " - current size = " << Size() << "\n";
    if (Capacity() == 0) {
      ss << "[ EMPTY ]";
      return ss.str();
    }
    
    ss << " - number elements = " << NumElements() << "\n";

    size_t idx = 0 , cursor = 0;
    for (auto& elt_size : element_sizes) {

      ss << std::dec << " -- [" << idx << " : " << elt_size << "] = ";
      ss << std::hex;
      for (uint32_t i = cursor; i < cursor + elt_size; ++i) {
        ss << "0x";
        if (static_cast<uint64_t>(data[i]) < 16) {
          ss << "0";
        }
        ss << static_cast<uint64_t>(data[i]) << " ";
      }
      ss << "\n";

      ++idx;
      cursor += elt_size;
    }

    return ss.str();
  }

  uint64_t Buffer::Size() const {
    return std::accumulate(element_sizes.begin() , element_sizes.end() , 0);
  }
      
  uint64_t Buffer::Capacity() const {
    return capacity;
  }
      
  uint64_t Buffer::NumElements() const {
    return element_sizes.size();
  }
  
  void Buffer::SetUniformElementSize(uint64_t num_elts , uint64_t size) {
    element_sizes.clear();
    element_sizes.resize(num_elts);
    std::ranges::fill(element_sizes , size);
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
