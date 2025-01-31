/**
 * \file core/byte_buffer.cpp
 **/
#include "core/byte_buffer.hpp"

namespace other {

  std::string ByteBuffer::DumpBuffer() const {
    std::stringstream ss;

    if (data == nullptr) {
      ss << "[ EMPTY ]";
      return ss.str();
    }
    ss << "Bytes written = " << bytes_written << "\n";

    for (uint32_t i = 0; i < bytes_written; ++i) {
      using namespace std::string_view_literals;
      ss << fmt::format("{:#02x} "sv, data[i]);
    }
    ss << "\n";

    return ss.str();
  }

}  // namespace other