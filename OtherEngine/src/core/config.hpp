/**
 * \file core\config.hpp
 */
#ifndef OTHER_ENGINE_CONFIG_HPP
#define OTHER_ENGINE_CONFIG_HPP

#include <map>
#include <vector>
#include <string>

#include "core/defines.hpp"

namespace other {

  class ConfigTable {
    public:
      ConfigTable() = default;
      ~ConfigTable() = default;

      void Add(const std::string& section, const std::string& key = "", const std::string& value = "" , bool is_string = false);

      const std::map<uint64_t , std::vector<std::string>> Get(const std::string& section) const;
      const std::vector<std::string> Get(const std::string& section, const std::string& key) const;
      const std::vector<std::string> GetKeys(const std::string& section) const;
      
      template <typename T>
      const Opt<T> GetVal(const std::string& section, const std::string& key) const;

      std::string TableString();

    private:
      std::map<uint64_t , std::string> section_map;
      std::map<uint64_t , std::string> key_map;
      std::map<uint64_t , std::vector<std::string>> key_names;
      std::map<uint64_t , std::map<uint64_t , std::vector<std::string>>> table;
  };

} // namespace other

#endif // !OTHER_ENGINE_CONFIG_HPP
