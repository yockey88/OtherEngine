/**
 * \file core/writer_reader.hpp
 **/
#ifndef OTHER_ENGINE_WRITER_READER_HPP
#define OTHER_ENGINE_WRITER_READER_HPP

#include <cctype>
#include <concepts>
#include <istream>
#include <ostream>
#include <type_traits>

#include <glm/glm.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <reflection/reflected_object.hpp>

#include "core/defines.hpp"
#include "core/meta.hpp"

namespace other {

  void ClearWhitespace(std::istream& is);
  void BeginReadList(std::istream& os);
  void EndReadList(std::istream& os);

  std::ostream& BeginWriteList(std::ostream& os);
  std::ostream& EndWriteList(std::ostream& os);

  bool IsMarkerChar(char c);
  bool IsSentinelChar(char c);

  class WriteError : public std::runtime_error {
   public:
    explicit WriteError(const std::string& msg)
        : std::runtime_error(msg) {}
  };

  class ReadError : public std::runtime_error {
   public:
    explicit ReadError(const std::string& msg)
        : std::runtime_error(msg) {}
  };

  template <typename T>
  concept Appendable = requires(T t, char c) {
    t.append(1, c);
  };

  template <typename T>
  struct Writer;

  template <typename T>
  struct JsonWriter;

  template <typename T>
  struct Reader;

  template <typename T>
  concept InStreamable =
    requires(const T& data) { std::declval<std::istream&>() >> data; };

  template <typename T>
  concept OutStreamable =
    requires(const T& data) { std::declval<std::ostream&>() << data; };

  template <typename T>
  concept Writable =
    requires(const T& data) {
      { Writer<T>{}(std::declval<std::ostream&>(), data) } -> std::same_as<std::ostream&>;
    };

  template <typename T>
  concept JsonWritable =
    requires(std::ostream& os, const T& data) {
      { Writer<T>{}(data) } -> std::convertible_to<nlohmann::json>;
    };

  template <typename T>
  concept Readable = requires(std::istream& os, const T& data) {
    { Reader<T>{}(os) } -> std::convertible_to<T>;
  };

  template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
  struct Writer<T> {
    std::ostream& operator()(std::ostream& stream, T data) const {
      stream << data;
      return stream;
    }
  };

  template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
  struct Reader<T> {
    T operator()(std::istream& stream) const {
      ClearWhitespace(stream);

      T data;
      stream >> data;
      return data;
    }
  };

  template <>
  struct Reader<char> {
    char operator()(std::istream& os) const {
      try {
        ClearWhitespace(os);
        if (os.fail() || os.eof()) {
          return '\0';
        }

        return os.get();
      } catch (...) {
        return '\0';
      }
    }
  };

  template <typename CT>
    requires is_container<CT> && Writable<typename CT::value_type>
  struct Writer<CT> {
    std::ostream& operator()(std::ostream& stream, const CT& data) const {
      if constexpr (!std::same_as<CT, std::string> && !std::same_as<CT, std::string_view>) {
        BeginWriteList(stream) << "\n      ";
      }
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        Writer<typename CT::value_type>{}(stream, *itr);
        if constexpr (!std::same_as<CT, std::string> && !std::same_as<CT, std::string_view>) {
          if (itr != std::prev(data.end())) {
            Writer<char>{}(stream, ',') << "\n      ";
          } else {
            stream << "\n  ";
          }
        }
      }
      if constexpr (!std::same_as<CT, std::string> && !std::same_as<CT, std::string_view>) {
        EndWriteList(stream) << "\n";
      }
      return stream;
    }
  };

  template <typename T>
  concept StringType = std::same_as<T, std::string> || std::same_as<T, std::string_view>;
  template <typename T>
  concept NotStringType = !StringType<T>;

  template <typename CT>
    requires NotStringType<CT> && is_container<CT> && Readable<typename CT::value_type>
  struct Reader<CT> {
    CT operator()(std::istream& stream) const {
      using val_t = typename CT::value_type;

      ClearWhitespace(stream);

      CT data;
      if (stream.peek() == '{' || stream.peek() == '[' || stream.peek() == '(') {
        stream.ignore();
      }

      while (!stream.fail() && !stream.eof() &&
             !(stream.peek() == '}' || stream.peek() == ']' || stream.peek() == ')')) {
        data.push_back(Reader<val_t>{}(stream));

        ClearWhitespace(stream);
        if (stream.peek() == ',') {
          stream.ignore();
        }
      }

      if (stream.peek() == '}' || stream.peek() != ']' || stream.peek() != ')' ||
          IsSentinelChar(stream.peek())) {
        stream.ignore();
      }

      return data;
    }
  };

  template <typename T>
    requires StringType<T>
  struct Reader<T> {
    T operator()(std::istream& stream) const {
      ClearWhitespace(stream);
      if (stream.eof()) {
        return "";
      }

      std::string res = "";
      char c = stream.peek();
      while (!stream.eof() && !IsSentinelChar(c) && (std::isalpha(c) || std::isdigit(c) || c == '_' || c == '-')) {
        res.append(1, stream.get());
        c = stream.peek();
      }

      return res;
    }
  };

  template <>
  struct Writer<bool> {
    std::ostream& operator()(std::ostream& stream, const bool& data) const {
      stream << (data ? "true" : "false");
      return stream;
    }
  };

  template <>
  struct Reader<bool> {
    bool operator()(std::istream& stream) const {
      std::string data = Reader<std::string>{}(stream);
      return data == "true";
    }
  };

  template <typename T>
    requires Writable<T>
  std::ostream& WriteListItem(std::ostream& os, const T& data, bool is_last = false) {
    Writer<T>{}(os, data);
    if (!is_last) {
      os << ",";
    }
    return os;
  }

  template <typename T>
    requires Readable<T>
  T ReadListItem(std::istream& is, Reader<T>&& reader) {
    while (!is.eof() && std::isspace(is.peek())) {
      is.ignore();
    }
    T read = reader(is);
    while (!is.eof() && std::isspace(is.peek())) {
      is.ignore();
    }
    if (is.peek() == ',') {
      is.ignore();
    }
    return read;
  }

  template <typename T>
    requires Writable<T>
  std::ostream& WriteKeyValue(std::ostream& os, const std::string& key, const T& data) {
    Writer<std::string>{}(os, key);
    os << " = ";
    Writer<T>{}(os, data);
    os << "\n";
    return os;
  }

  template <typename T>
    requires Readable<T>
  std::pair<std::string, T> ReadKeyValue(std::istream& is, Reader<T>&& reader) {
    std::string key = Reader<std::string>{}(is);
    ClearWhitespace(is);

    Reader<char>{}(is);  /// '='
    ClearWhitespace(is);

    T read = reader(is);
    return { key, read };
  }

  template <>
  struct Writer<ValueType> {
    std::ostream& operator()(std::ostream& os, const ValueType& data) {
      os << ValueTypeToString(data);
      return os;
    }
  };

  template <>
  struct Reader<ValueType> {
    ValueType operator()(std::istream& is) {
      while (std::isspace(is.peek())) {
        is.ignore();
      }

      std::string type_str;
      while (!is.eof() && (std::isalpha(is.peek()) || std::isdigit(is.peek()) || is.peek() == '_' || is.peek() == '-')) {
        type_str.append(1, is.get());
      }
      if (is.peek() == ':') {
        is.ignore();
      }

      return StringToValueType(type_str);
    }
  };

  template <typename T>
  struct Writer<glm::tvec2<T>> {
    std::ostream& operator()(std::ostream& stream, const glm::tvec2<T>& data) const {
      BeginWriteList(stream);
      WriteListItem(stream, data.x);
      WriteListItem(stream, data.y, true);
      EndWriteList(stream);
      return stream;
    }
  };

  template <typename T>
  struct Reader<glm::tvec2<T>> {
    glm::tvec2<T> operator()(std::istream& stream) const {
      glm::tvec2<T> data;
      BeginReadList(stream);
      data.x = ReadListItem(stream, Reader<T>{});
      data.y = ReadListItem(stream, Reader<T>{});
      EndReadList(stream);
      return data;
    }
  };

  template <typename T>
  struct Writer<glm::tvec3<T>> {
    std::ostream& operator()(std::ostream& stream, const glm::tvec3<T>& data) const {
      BeginWriteList(stream);
      WriteListItem(stream, data.x);
      WriteListItem(stream, data.y);
      WriteListItem(stream, data.z, true);
      EndWriteList(stream);
      return stream;
    }
  };

  template <typename T>
  struct Reader<glm::tvec3<T>> {
    glm::tvec3<T> operator()(std::istream& stream) const {
      glm::tvec3<T> data;
      BeginReadList(stream);
      data.x = ReadListItem(stream, Reader<T>{});
      data.y = ReadListItem(stream, Reader<T>{});
      data.z = ReadListItem(stream, Reader<T>{});
      EndReadList(stream);
      return data;
    }
  };

  template <typename T>
  struct Writer<glm::tvec4<T>> {
    std::ostream& operator()(std::ostream& stream, const glm::tvec4<T>& data) const {
      BeginWriteList(stream);
      WriteListItem(stream, data.x);
      WriteListItem(stream, data.y);
      WriteListItem(stream, data.z);
      WriteListItem(stream, data.w, true);
      EndWriteList(stream);
      return stream;
    }
  };

  template <typename T>
  struct Reader<glm::tvec4<T>> {
    glm::tvec4<T> operator()(std::istream& stream) const {
      glm::tvec4<T> data;
      BeginReadList(stream);
      data.x = ReadListItem(stream, Reader<T>{});
      data.y = ReadListItem(stream, Reader<T>{});
      data.z = ReadListItem(stream, Reader<T>{});
      data.w = ReadListItem(stream, Reader<T>{});
      EndReadList(stream);
      return data;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_WRITER_READER_HPP