/**
 * \file core/platform.hpp
 */
#ifndef OTHER_ENGINE_PLATFORM_HPP
#define OTHER_ENGINE_PLATFORM_HPP

#include "core/defines.hpp"

#ifdef _WIN32
  #define OE_WINDOWS
#elif __linux__
  #define OE_LINUX
#elif __APPLE__
  #define OE_MAC
#else
  #error "Unsupported platform"
#endif

#ifdef OE_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>

  #ifdef OE_CLIENT
    #define OE_API extern "C" __declspec(dllexport)
  #else
    #define OE_API extern "C" __declspec(dllimport)
  #endif
#else
  #ifdef OE_CLIENT
    #define OE_API __attribute__((visibility("default"))) 
  #else
    #define OE_API
  #endif 
#endif

#endif // !OTHER_ENGINE_PLATFORM_HPP
