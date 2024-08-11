/**
 * \file core/buffer.hpp
 **/
#ifndef OTHER_ENGINE_BUFFER_HPP
#define OTHER_ENGINE_BUFFER_HPP

#include <cstdint>
#include <array>
#include <numeric>
#include <span>

#include <glm/gtc/type_ptr.hpp>

#include "core/logger.hpp"
#include "math/vecmath.hpp"

namespace other {

  template <typename T , typename U , size_t N>
    concept ContainerType = std::same_as<T , std::span<U , N>> || std::same_as<T , std::array<U , N>> ||
                            std::same_as<T , std::vector<U>>;

  class Buffer {
    public:
      Buffer() 
        : data(nullptr) , capacity(0) {}
      Buffer(void* data , uint64_t size);

      Buffer(Buffer&& other);
      Buffer(const Buffer& other);
      Buffer& operator=(Buffer&& other);
      Buffer& operator=(const Buffer& other);

      void Allocate(uint64_t size);
      void Extend();
      void Release();
      void ZeroMem();

      size_t ElementSize(size_t index) const;

      template <typename T>
      T& Read(uint64_t offset = 0) {
        OE_ASSERT(offset + sizeof(T) <= capacity , "Attempting to read buffer as incorrectly sized type");
        return *reinterpret_cast<T*>(data + offset);
      }

      template <typename T>
      const T& Read(uint64_t offset = 0) const {
        OE_ASSERT(offset + sizeof(T) <= capacity , "Attempting to read buffer as incorrectly sized type");
        return *reinterpret_cast<T*>(data + offset);
      }

      const uint8_t* ReadBytes(uint64_t offset = 0) const;
      void Write(const void* data , uint64_t size , uint64_t offset = 0);

      template <typename T>
      void Write(const T& value) {
        if constexpr (std::same_as<T , std::string> || std::same_as<T , std::string_view>) {
          Allocate(value.length() + 1);
          for (size_t i = 0; i < value.length(); ++i) {
            data[i] = static_cast<uint8_t>(value[i]);
          }
          data[value.length()] = '\0';
          SetUniformElementSize(1, value.length());
        } else {
          size_t sz = sizeof(value);
          Allocate(sz);
          if constexpr (glm_t<T>) {
            Write(glm::value_ptr(value) , sz , Size());
          } else {
            Write(&value , sz , Size());
          }
          SetUniformElementSize(1, sz);
        }
      }

      template <typename T>
      void BufferData(const T& value) {
        if (Capacity() == 0) {
          if constexpr (std::same_as<T , std::string> || std::same_as<T , std::string_view>) {
            /// arbitrary, but assume use of this function means we're gonna want more than one 
            Allocate((value.length()+ 1) * 64);
          } else {
            Allocate(sizeof(value) * 64);
          }
        }

        if constexpr (std::same_as<T , std::string> || std::same_as<T , std::string_view>) {
          if (Size() + value.length() > Capacity()) {
            Extend(); 
          }
          for (size_t i = 0; i < value.length(); ++i) {
            data[Size() + i] = static_cast<uint8_t>(value[i]);
          }
          data[Size() + value.length()] = '\0';
          element_sizes.push_back(value.length());
        } else {
          if (Size() + sizeof(value) > Capacity()) {
            Extend(); 
          }
          
          size_t sz = sizeof(value);
          T& obj = *reinterpret_cast<T*>(data + Size());
          obj = value;

          element_sizes.push_back(sz);
        }
      }
      
      template <typename T>
      void WriteAt(const T& value , size_t index) {
      }

      template <typename U>
      void WriteArr(const std::span<U>& container , size_t start_index = 0 , size_t num_elts = 0) {
        if (num_elts == 0) {
          num_elts = container.size();
        }

        if (start_index + num_elts > container.size()) {
          OE_ERROR("Attempting to read from invalid memory when writing to buffer! expected min {} > {} real size" ,
                    start_index + num_elts , container.size());
          return;
        }

        Allocate(num_elts * sizeof(U));
        SetUniformElementSize(num_elts , sizeof(U));

        size_t cursor = 0;
        for (size_t i = start_index; i < start_index + num_elts; ++i) {
          U current_item = container[i]; 
          *reinterpret_cast<U*>(data + cursor) = current_item;
          cursor += sizeof(U);
        }
      }

      operator bool() const {
        return data != nullptr;
      }

      uint8_t& operator[](uint64_t offset);
      uint8_t operator[](uint64_t offset) const;

      template <typename T>
      const T* As() const {
        OE_ASSERT(sizeof(T) <= capacity , "Attempting to retrieve data is incorrectly sized type");
        return reinterpret_cast<T*>(&data[0]);
      }

      template <typename T>
      T& At(size_t index) {
        OE_ASSERT(sizeof(T) <= capacity , "Attempting to retrieve data with incorrectly sized type! sizeof({}) == {} > {}" , 
                  typeid(T).name() , sizeof(T) , capacity);
        OE_ASSERT(index < element_sizes.size() , "Attempting to retrieve invalid index! expected {} > {} num elements" , index , element_sizes.size());
        OE_ASSERT(sizeof(T) == element_sizes[index] , "Attempting to access buffer with invalidly sized type {}! expected size {} != {} stored size" ,
                  typeid(T).name() , sizeof(T) , element_sizes[index]);
        size_t offset = std::accumulate(element_sizes.begin() , element_sizes.begin() + index , 0);
        if (glm_t<T>) {
          return *reinterpret_cast<T*>((float*)(data + offset));
        } else {
          return *reinterpret_cast<T*>(data + offset);
        }
      };
      
      template <typename T>
      const T& At(size_t index) const {
        return At<T>(index);
      };

      std::string DumpBuffer() const;

      uint64_t Size() const;
      uint64_t Capacity() const;
      uint64_t NumElements() const;

    protected:
      uint8_t* data = nullptr;
      uint64_t capacity = 0;
      std::vector<uint64_t> element_sizes;

      void SetUniformElementSize(uint64_t num_elts , uint64_t size);
  };

  struct SafeBuffer : public Buffer {
    ~SafeBuffer();
    static SafeBuffer Copy(const SafeBuffer& other);
  };

} // namespace other

#endif // !OTHER_ENGINE_BUFFER_HPP
