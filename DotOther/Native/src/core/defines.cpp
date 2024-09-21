/**
 * \file core/defines.cpp
 **/
#include "core/defines.hpp"

namespace dotother {
namespace util {

#ifdef DOTOTHER_WIDE_CHARS
  std::wstring CharToWide(const std::string_view str) {
    int32_t length = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int32_t>(str.length()), nullptr, 0);
    std::wstring res(length, wchar_t{0});
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int32_t>(str.length()), res.data(),length);
    return res;
  }
  
  std::string WideToChar(const std::wstring_view str) {
    int32_t length = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int32_t>(str.length()), nullptr, 0, nullptr, nullptr);
    std::string res(length, char{0});
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int32_t>(str.length()), res.data(), length, nullptr, nullptr);
    return res;
  }
	
#else
  std::string CharToWide(const std::string_view str) {
    return std::string(str);
  }
  
  std::string WideToChar(const std::wstring_view str) {
    return std::string(str);
  }
#endif // DOTOTHER_WIDE_CHARS

} // namespace util
} // namespace dotother
