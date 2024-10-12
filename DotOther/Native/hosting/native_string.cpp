/**
 * \file hosting/native_string.cpp
 */
#include "hosting/native_string.hpp"

#include "core/utilities.hpp"
#include "hosting/memory.hpp"

namespace dotother {

  NString NString::New(const char* str) {
    NString result;
    result.Assign(str);
    return result;
  }

  NString NString::New(std::string_view str) {
    NString result;
    result.Assign(str);
    return result;
  }
  
  void NString::Free(NString& str) {
    if (str.string == nullptr)
      return;

    Memory::FreeCoTaskMem(str.string);
    str.string = nullptr;
  }

  void NString::Assign(std::string_view str) {
    if (string != nullptr)
      Memory::FreeCoTaskMem(string);

    string = Memory::NStringToCoTaskMemAuto(util::CharToWide(str));
  }

  NString::operator std::string() const {
    dostring_view str(string);
    return 
#ifdef _WIN32
      util::WideToChar(str);
#else
      std::string(str);
#endif // _WIN32
  }

  bool NString::operator==(const NString& InOther) const {
    if (string == InOther.string)
      return true;

    if (string == nullptr || InOther.string == nullptr)
      return false;

    return wcscmp(string, InOther.string) == 0;
  }

  bool NString::operator==(std::string_view InOther) const {
    // auto str = NStringHelper::ConvertUtf8ToWide(InOther);
    // return wcscmp(m_NString, str.data()) == 0;
    return false;
  }

  wchar_t* NString::Data() { 
    return string; 
  }

  const wchar_t* NString::Data() const { 
    return string; 
  }

  
  NScopedString::~NScopedString() {
    NString::Free(string);
  }

  NScopedString& NScopedString::operator=(NString str) {
    NString::Free(string);
    string = str;
    return *this;
  }

  NScopedString& NScopedString::operator=(const NString& str) {
    NString::Free(string);
    string = str;
    return *this;
  }

  NScopedString::operator std::string() const {
    return string;
  }

  NScopedString::operator NString() const {
    return string;
  }

  bool NScopedString::operator==(const NScopedString& other) const {
    return string == other.string;
  }

  bool NScopedString::operator==(std::string_view other) const {
    return string == other;
  }

} // namespace dotother