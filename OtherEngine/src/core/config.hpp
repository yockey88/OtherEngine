/**
 * \file core\config.hpp
 */
#ifndef OTHER_ENGINE_CONFIG_HPP
#define OTHER_ENGINE_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

#include "core/defines.hpp"

namespace other {

  class ConfigTable {
   public:
    ConfigTable() = default;
    ~ConfigTable() = default;

    void Add(const std::string_view section, const std::string_view key = "", const std::string_view value = "", bool is_string = false, bool allow_key_modifications = true);
    void Add(const std::string_view section, const std::string_view key, const std::vector<std::string>& list, bool is_string = false, bool allow_key_modifications = true);

    const std::map<uint64_t, std::vector<std::string>> Get(const std::string_view section) const;
    const std::vector<std::string> Get(const std::string_view section, const std::string_view key, bool case_sensitive_key = false) const;
    const std::vector<std::string> GetKeys(const std::string_view section) const;

    template <typename T>
    const Opt<T> GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const;

    std::string TableString();

   private:
    std::map<uint64_t, std::string> section_map;
    std::map<uint64_t, std::string> key_map;
    std::map<uint64_t, std::vector<std::string>> key_names;
    std::map<uint64_t, std::map<uint64_t, std::vector<std::string>>> table;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_CONFIG_HPP
