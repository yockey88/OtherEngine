/**
 * \file  memory.cpp
 **/
#include "memory.hpp"

#include "defines.hpp"

namespace dotother {

  void* Memory::AllocHGlobal(size_t InSize) {
#if defined(_WIN32)
    return LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, InSize);
#else
    return malloc(InSize);
#endif
  }

  void Memory::FreeHGlobal(void* InPtr) {
#if defined(_WIN32)
    LocalFree(InPtr);
#else
    free(InPtr);
#endif
  }

// 	  static wchar_t* StringToCoTaskMemAuto(std::string_view InString) {
// 		  size_t length = InString.length() + 1;
// 		  size_t size = length * sizeof(wchar_t);

// #if defined(_WIN32)
// 		  auto* buffer = static_cast<wchar_t*>(CoTaskMemAlloc(size));

//       if (buffer != nullptr) {
//         memset(buffer, 0xCE, size);
//         wcscpy(buffer, InString.data());
//       }
// #else
//       auto* buffer = static_cast<wchar_t*>(AllocHGlobal(size));

//       if (buffer != nullptr) {
//         memset(buffer, 0, size);
//         strcpy(buffer, InString.data());
//       }
// #endif
// 		  return buffer;
//   	}

  void Memory::FreeCoTaskMem(void* InMemory) {
#if defined(_WIN32)
    CoTaskMemFree(InMemory);
#else
    FreeHGlobal(InMemory);
#endif
  }

} // namespace dotother