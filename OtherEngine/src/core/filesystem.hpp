/**
 * \file core/filesystem.hpp
 **/
#ifndef OTHER_ENGINE_FILESYSTEM_HPP
#define OTHER_ENGINE_FILESYSTEM_HPP

#include <string>
#include <string_view>
#include <filesystem>

namespace other {

   class Filesystem {
     public:
       static bool FileExists(const std::string& path);
       static bool FileExists(const std::string_view path);
       static bool FileExists(const std::filesystem::path& path);

       static std::vector<char> ReadFileAsChars(const std::string& path);
   };

} // namespace other

#endif // !OTHER_ENGINE_FILESYSTEM_HPP
