/**
 * \file core/writer_reader.hpp
 **/
#ifndef OTHER_ENGINE_WRITER_READER_HPP
#define OTHER_ENGINE_WRITER_READER_HPP

#include <concepts>
#include <istream>
#include <ostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "core/meta.hpp"

namespace other {

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

  template <>
  struct Writer<char> {
    std::ostream& operator()(std::ostream& stream, char data) const {
      stream << std::string(1, data);
      return stream;
    }
  };

  template <>
  struct Writer<std::string> {
    std::ostream& operator()(std::ostream& stream, const std::string& data) const {
      stream << data;
      return stream;
    }
  };

  template <typename T>
    requires std::same_as<T, std::string> || std::same_as<T, std::string_view>
  struct Reader<T> {
    T operator()(std::istream& os) const {
      try {
        if (os.fail() || os.eof()) {
          return T{};
        }

        T data;
        os >> data;
        return data;
      } catch (...) {
        return T{};
      }
    }
  };

  template <>
  struct Reader<char> {
    char operator()(std::istream& os) const {
      try {
        if (os.fail() || os.eof()) {
          return '\0';
        }

        std::string temp(1, char(os.get()));
        if (temp.size() != 1) {
          return '\0';
        }

        return temp[0];
      } catch (...) {
        return '\0';
      }
    }
  };

  template <typename CT>
    requires is_container<CT> && Writable<typename CT::value_type>
  struct Writer<CT> {
    std::ostream& operator()(std::ostream& stream, const CT& data) const {
      std::stringstream ss;
      for (const auto& val : data) {
        Writer<typename CT::value_type>{}(stream, val);
        Writer<char>{}(stream, '\n');
      }
    }
  };

  template <typename CT>
    requires(is_container<CT> && !std::same_as<CT, std::string> && !std::same_as<typename CT::value_type, char>) &&
    Readable<typename CT::value_type>
  struct Reader<CT> {
    CT operator()(std::istream& stream) const {
      using val_t = typename CT::value_type;

      CT data;
      val_t val;
      while (!stream.fail() && !stream.eof()) {
        data.push_back(Reader<val_t>{}(stream));
      }

      return data;
    }
  };

  // template <typename T>
  //   requires Writable<T> && (!std::same_as<T, std::string> && !std::same_as<T, std::string_view>)
  // std::ostream& operator<<(std::ostream& os, const T& data) {
  //   return Writer<T>{}(os, data);
  // }

}  // namespace other

#endif  // !OTHER_ENGINE_WRITER_READER_HPP