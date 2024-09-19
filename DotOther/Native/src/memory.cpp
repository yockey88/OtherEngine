/**
 * \file  memory.cpp
 **/
#include "memory.hpp"

namespace dotother {

  void* Memory::AllocHGlobal(size_t InSize) {
#ifdef _WIN32
    return LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, InSize);
#else
    return malloc(InSize);
#endif // _WIN32
  }

  void Memory::FreeHGlobal(void* InPtr) {
#ifdef _WIN32
    LocalFree(InPtr);
#else
    free(InPtr);
#endif // _WIN32
  }

  dochar* Memory::NStringToCoTaskMemAuto(dostring_view str) {
    size_t length = str.length() + 1;
    size_t size = length * sizeof(dochar);

    auto* buffer = static_cast<dochar*>(
#ifdef _WIN32
      CoTaskMemAlloc(size)
#else
      AllocHGlobal(size)
#endif // _WIN32
      );

    if (buffer != nullptr) {
#ifdef _WIN32
      memset(buffer, 0xCE, size);
      wcscpy(buffer, str.data());
#else
      memset(buffer, 0, size);
      strcpy(buffer, str.data());
#endif // _WIN32
    }
    return buffer;
  }

  void Memory::FreeCoTaskMem(void* InMemory) {
#ifdef _WIN32
    CoTaskMemFree(InMemory);
#else
    FreeHGlobal(InMemory);
#endif // _WIN32
  }

} // namespace dotother