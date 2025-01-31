/**
 * \file serialization/object_serializer.hpp
 **/
#ifndef OTHER_ENGINE_OBJECT_SERIALIZER_HPP
#define OTHER_ENGINE_OBJECT_SERIALIZER_HPP

#include <functional>

#include "core/byte_buffer.hpp"

namespace other {

  template <typename T, size_t NFs>
  class ObjectSerializer {
   public:
    ObjectSerializer() = default;
    ~ObjectSerializer() = default;

    void Write(ByteBuffer& stream, const T& object) {
      for (auto& writer : writers) {
        OE_ASSERT(writer != nullptr, "Writer is null");
        writer(stream, object);
      }
    }

    T Read(ByteBuffer& stream, size_t buffer_offset) {
      T obj;

      size_t offset = buffer_offset;
      for (auto& reader : readers) {
        if (reader == nullptr) {
          continue;
        }
        reader(stream, offset, obj);
      }
      return obj;
    }

    static constexpr size_t NumFields() { return NFs; }

    using Writer = std::function<void(ByteBuffer&, const T& object)>;
    using Reader = std::function<void(ByteBuffer&, size_t&, T&)>;

   protected:
    template <typename FT, size_t FN>
    void AddField(FT T::*field) {
      static_assert(FN < NFs, "Field number out of bounds");
      OE_ASSERT(field != nullptr, "Field pointer is null");

      writers[FN] = [=](ByteBuffer& buffer, const T& object) {
        buffer.BufferData<FT>(object.*field);
      };
      readers[FN] = [=](ByteBuffer& buffer, size_t& buffer_offset, T& obj) {
        obj.*field = buffer.Read<FT>(buffer_offset);
        buffer_offset += sizeof(FT);
      };
    }

   private:
    std::array<Writer, NFs> writers = { nullptr };
    std::array<Reader, NFs> readers = { nullptr };
  };

}  // namespace other

#endif  // !OTHER_ENGINE_OBJECT_SERIALIZER_HPP