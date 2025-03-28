/**
 * \file serialization/object_serializer.hpp
 **/
#ifndef OTHER_ENGINE_OBJECT_SERIALIZER_HPP
#define OTHER_ENGINE_OBJECT_SERIALIZER_HPP

#include <functional>

#include <refl/refl.hpp>
#include <reflection/serializable.hpp>

#include "core/byte_buffer.hpp"
#include "core/logger.hpp"

#include "serialization/type_meta.hpp"

namespace other {

  template <typename T>
  void Serialize(ByteBuffer& buffer, T&& value) {
    refl::util::for_each(refl::reflect(value).members, [&](auto member) {
      if constexpr (refl::descriptor::is_readable(member) &&
                    refl::descriptor::has_attribute<dotother::echo::serializable_field>(member)) {
        buffer.BufferData(member(value));
      }
    });
  }

  template <typename T, size_t NFs>
  class ObjectSerializer {
   public:
    ObjectSerializer() = default;
    ~ObjectSerializer() = default;

    virtual void Write(ByteBuffer& stream, const T& object) {
      for (auto& writer : writers) {
        OE_ASSERT(writer != nullptr, "Writer is null");
        writer(stream, object);
      }
    }

    virtual T Read(ByteBuffer& stream, size_t buffer_offset) {
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

      AddReaderWriterForField<FT, FN>(
        [=](ByteBuffer& buffer, const T& object) {
          buffer.BufferData<FT>(object.*field);
        },
        [=](ByteBuffer& buffer, size_t& buffer_offset, T& obj) {
          obj.*field = buffer.Read<FT>(buffer_offset);
          buffer_offset += sizeof(FT);
        }
      );
    }

    template <typename FT, size_t FN>
    void AddReaderWriterForField(Writer writer, Reader reader) {
      static_assert(FN < NFs, "Field number out of bounds");
      OE_ASSERT(writer != nullptr, "Writer is null");
      OE_ASSERT(reader != nullptr, "Reader is null");

      writers[FN] = writer;
      readers[FN] = reader;
    }

   private:
    std::array<Writer, NFs> writers = { nullptr };
    std::array<Reader, NFs> readers = { nullptr };
  };

}  // namespace other

#endif  // !OTHER_ENGINE_OBJECT_SERIALIZER_HPP
