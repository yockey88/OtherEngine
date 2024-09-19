/**
 * \file native_string.hpp
 **/
#ifndef DOTOTHER_NATIVE_STRING_HPP
#define DOTOTHER_NATIVE_STRING_HPP

#include <string>
#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "defines.hpp"

namespace dotother {

  class NString {
  public:
    static NString New(const char* str);
    static NString New(std::string_view str);
    static void Free(NString& str);

    void Assign(std::string_view str);

    operator std::string() const;

    bool operator==(const NString& other) const;
    bool operator==(std::string_view other) const;

    wchar_t* Data();
    const wchar_t* Data() const;

  private:
    wchar_t* string = nullptr;
    dobool disposed = false;
};

  class NScopedString {
    public:
      NScopedString(NString string) 
        : string(string) {}
      ~NScopedString();

      NScopedString& operator=(NString str);
      NScopedString& operator=(const NString& str);

      operator std::string() const;
      operator NString() const;

      bool operator==(const NScopedString& other) const;
      bool operator==(std::string_view other) const;

    private:
      NString string;
  };

} // namespace dotother

template <>
struct fmt::formatter<dotother::NString> : public fmt::formatter<std::string_view> {
  auto format(const dotother::NString& str, fmt::format_context& ctx) {
    std::string s = str;
    return fmt::formatter<std::string_view>::format(s, ctx);
  }
};

#endif // !DOTOTHER_NATIVE_STRING_HPP