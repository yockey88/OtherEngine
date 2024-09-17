/**
 * \file  native_string.cpp
 */
#include "native_string.hpp"

#include "memory.hpp"


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

    // string = Memory::NStringToCoTaskMemAuto(NStringHelper::ConvertUtf8ToWide(str));
  }

  NString::operator std::string() const {
    std::wstring_view str(string);
    return ""; // NStringHelper::ConvertWideToUtf8(str);
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

} // namespace dotother