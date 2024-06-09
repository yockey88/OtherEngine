/**
 * \file core/buffer.hpp
 **/
#ifndef OTHER_ENGINE_BUFFER_HPP
#define OTHER_ENGINE_BUFFER_HPP

#include <cstdint>
#include <typeinfo>

#include "core/logger.hpp"

namespace other {

  class Buffer {
    public:
      Buffer() {}
      Buffer(void* data , uint64_t size);

      Buffer(Buffer&& other);
      Buffer(const Buffer& other);
      Buffer& operator=(Buffer&& other);
      Buffer& operator=(const Buffer& other);

      static Buffer Copy(const Buffer& other);

      static Buffer Copy(const void* data , uint64_t size);

      void Allocate(uint64_t size);

      void Release();

      void ZeroMem();

      template <typename T>
      T& Read(uint64_t offset = 0) {
        OE_ASSERT(offset + sizeof(T) <= size , "Attempting to read buffer as incorrectly sized type");
        return *reinterpret_cast<T*>(data + offset);
      }

      template <typename T>
      const T& Read(uint64_t offset = 0) const {
        OE_ASSERT(offset + sizeof(T) <= size , "Attempting to read buffer as incorrectly sized type");
        return *reinterpret_cast<T*>(data + offset);
      }

      const uint8_t* ReadBytes(uint64_t offset) const;

      void Write(const void* data , uint64_t size , uint64_t offset = 0);

      template <typename T>
      void Write(const T& value) {
        OE_ASSERT(sizeof(data) >= size , "Buffer::Write({}) Out of bounds" , typeid(T).name());
        memcpy(data , &value , sizeof(data));
        size = sizeof(data);
      }

      operator bool() const {
        return data != nullptr;
      }

      uint8_t& operator[](uint64_t offset);
      uint8_t operator[](uint64_t offset) const;

      template <typename T>
      const T* As() const {
        OE_ASSERT(sizeof(T) == size , "Attempting to retrieve data is incorrectly sized type");
        return reinterpret_cast<T*>(&data[0]);
      }

      uint64_t Size() const;

    protected:
      uint8_t* data = nullptr;
      uint64_t size = 0;
  };

  struct SafeBuffer : public Buffer {
    ~SafeBuffer();
    static SafeBuffer Copy(const SafeBuffer& other);
  };

} // namespace other

#endif // !OTHER_ENGINE_BUFFER_HPP
