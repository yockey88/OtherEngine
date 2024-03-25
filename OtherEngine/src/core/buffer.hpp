/**
 * \file core/buffer.hpp
 **/
#ifndef OTHER_ENGINE_BUFFER_HPP
#define OTHER_ENGINE_BUFFER_HPP

#include <cstdint>

namespace other {

  class Buffer {
    public:
      Buffer() {}
      Buffer(void* data , uint64_t size)
        : data(data) , size(size) {}

      static Buffer Copy(const Buffer& other);

      static Buffer Copy(const void* data , uint64_t size);

      void Allocate(uint64_t size);

      void Release();

      void ZeroMem();

      template <typename T>
      T& Read(uint64_t offset) {
        return *(T*)((uint8_t*)data + offset);
      }

      template <typename T>
      const T& Read(uint64_t offset) const {
        return *(T*)((uint8_t*)data + offset);
      }

      uint8_t* ReadBytes(uint64_t size , uint64_t offset);

      void Write(const void* data , uint64_t size , uint64_t offset = 0);

      operator bool() const {
        return data != nullptr;
      }

      uint8_t& operator[](uint64_t offset);
      uint8_t operator[](uint64_t offset) const;

      template <typename T>
      T* As() {
        return (T*)data;
      }

      inline uint64_t Size() const;

    protected:
      void* data = nullptr;
      uint64_t size = 0;
  };

  struct SafeBuffer : public Buffer {
    ~SafeBuffer();
    static SafeBuffer Copy(const SafeBuffer& other);
  };

} // namespace other

#endif // !OTHER_ENGINE_BUFFER_HPP
