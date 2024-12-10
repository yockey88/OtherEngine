/**
 * \file core/writer_reader.cpp
 **/
#include "core/writer_reader.hpp"

namespace other {

  void ClearWhitespace(std::istream& is) {
    while (!is.eof() && (std::isspace(is.peek()) || IsSentinelChar(is.peek()))) {
      is.ignore();
    }
  }

  void BeginReadList(std::istream& is) {
    ClearWhitespace(is);
    if (is.peek() == '{') {
      is.ignore();
    }
  }

  void EndReadList(std::istream& is) {
    ClearWhitespace(is);
    if (is.peek() == '}') {
      is.ignore();
    }
  }

  std::ostream& BeginWriteList(std::ostream& os) {
    os << "{";
    return os;
  }

  std::ostream& EndWriteList(std::ostream& os) {
    os << "}";
    return os;
  }

  bool IsMarkerChar(char c) {
    static constexpr std::array markers = {
      '{', '}', '[', ']', '(', ')', '<', '>',
      ':', ';', '=', ',', '.', '!', '?', '@',
      '#', '$', '%', '^', '&', '*', '+', '=',
      '~', '`', '|', '\\', '/', '"', '\'', ' '
    };
    return std::ranges::find(markers, c) != markers.end();
  }

  bool IsSentinelChar(char c) {
    static constexpr std::array sentinels = {
      '\n', '\r', '\t', '\0', '\a', '\b', '\f', '\v'
    };
    return std::ranges::find(sentinels, c) != sentinels.end();
  }

}  // namespace other