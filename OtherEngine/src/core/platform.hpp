/**
 * \file core/platform.hpp
 */
#ifndef OTHER_ENGINE_PLATFORM_HPP
#define OTHER_ENGINE_PLATFORM_HPP

#include <array>
#include <filesystem>

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

namespace other {

  enum PlatformType {
    WINDOWS , 
    LINUX ,
    MAC ,

    NUM_PLATFORMS ,
    INVALID_PLATFORM = NUM_PLATFORMS
  };

  constexpr static uint64_t kNumPlatforms = PlatformType::NUM_PLATFORMS + 1;
  constexpr std::array<std::string_view, kNumPlatforms> kPlatforms = {
    "Windows" ,
    "Linux" ,
    "Mac" ,

    "Invalid"
  };

  enum LaunchType {
    EDITOR = 0 , 
    RUNTIME ,

    NUM_LAUNCH_TYPES ,
    INVALID_LAUNCH_TYPE = NUM_LAUNCH_TYPES
  };

  enum LaunchConfig {
    DEBUG = 0 ,
    RELEASE ,

    NUM_LAUNCH_CONFIGS ,
    INVALID_LAUNCH_CONFIG = NUM_LAUNCH_CONFIGS
  };

  constexpr static size_t kNumLaunchTypes = LaunchType::NUM_LAUNCH_TYPES + 1;
  constexpr static std::array<std::string_view , kNumLaunchTypes> kLaunchTypeNames = {
    "Editor" ,
    "Runtime" ,

    "Invalid"
  };

  class PlatformLayer {
    public:
      static PlatformType CurrentPlatform();

      static bool LaunchProject(const std::filesystem::path& path , LaunchType type);
      static bool LaunchProcess(const std::filesystem::path& path , const std::filesystem::path& working_dir , 
                                const std::string& args_str = "");
  };

} // namespace other

#endif // !OTHER_ENGINE_PLATFORM_HPP
