/**
 * \file native_string.hpp
 **/
#ifndef DOTOTHER_NATIVE_STRING_HPP
#define DOTOTHER_NATIVE_STRING_HPP

#include <string>
#include <string_view>

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

} // namespace dotother

#endif // !DOTOTHER_NATIVE_STRING_HPP