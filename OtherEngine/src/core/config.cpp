/**
 * \file core/config.cpp
 */
#include "core/config.hpp"

#include "core/logger.hpp"
#include "core/errors.hpp"

namespace other {

  void ConfigTable::Add(const std::string& section, const std::string& key, const std::string& value) {
    if (section.empty()) {
      return;
    }

    auto sec = section;
    std::transform(sec.begin() , sec.end() , sec.begin() , ::toupper);

    auto k = key;
    std::transform(k.begin() , k.end() , k.begin() , ::toupper);

    auto sec_hash = FNV(sec);
    auto key_hash = FNV(k);

    if (table.find(sec_hash) != table.end()) {
      if (key.empty()) {
        return;
      }

      if (value.empty()) {
        throw IniException("Value cannot be empty" , IniError::FILE_PARSE_ERROR);
      }

      if (table[sec_hash].find(key_hash) == table[sec_hash].end()) {
        key_map[key_hash] = k;
      }

      auto val = value;
      std::transform(val.begin() , val.end() , val.begin() , ::toupper);

      table[sec_hash][key_hash].push_back(val);
      return;
    } else {
      section_map[sec_hash] = sec;
      table[sec_hash] = std::map<uint64_t , std::vector<std::string>>();
    }

    if (key.empty()) {
      return;
    }

    if (value.empty()) {
      throw IniException("Value cannot be empty" , IniError::FILE_PARSE_ERROR);
    }

    if (table[sec_hash].find(key_hash) == table[sec_hash].end()) {
      key_map[key_hash] = k;
    }

    auto val = value;
    std::transform(val.begin() , val.end() , val.begin() , ::toupper);

    table[sec_hash][key_hash].push_back(val);
  }

  const std::map<uint64_t , std::vector<std::string>> ConfigTable::Get(const std::string& section) const {
    auto sec = section;
    std::transform(sec.begin() , sec.end() , sec.begin() , ::toupper);

    auto sec_hash = FNV(sec);

    if (table.find(sec_hash) != table.end()) {
      return table.at(sec_hash);
    }

    return {};
  }

  const std::vector<std::string> ConfigTable::Get(const std::string& section, const std::string& key) const {
    auto sec = section;
    std::transform(sec.begin() , sec.end() , sec.begin() , toupper);

    auto k = key;
    std::transform(k.begin() , k.end() , k.begin() , toupper);

    auto sec_hash = FNV(sec);
    auto key_hash = FNV(k);

    if (table.find(sec_hash) != table.end()) {
      if (table.at(sec_hash).find(key_hash) != table.at(sec_hash).end()) {
        std::vector<std::string> ret = table.at(sec_hash).at(key_hash);
        std::for_each(ret.begin() , ret.end() , [](std::string& s) {
            std::transform(s.begin() , s.end() , s.begin() , ::toupper);
        });
        return ret;
      }

      return {};
    }

    return {};
  }

  template <> const Opt<bool> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    if (ret_str.at(0) == "TRUE") {
      return true;
    } else if (ret_str.at(0) == "FALSE") {
      return false;
    }

    OE_ERROR("Invalid value for key : {}" , key);
    return std::nullopt;
  }

  template <> const Opt<uint8_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<uint16_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<uint32_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<uint64_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<int8_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<int16_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<int32_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}" , key);
      return std::nullopt;
    }
  }

  template <> const Opt<int64_t> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    Opt<int64_t> ret;
    try {
      ret = std::stoll(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}" , e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}" , e.what());
      return std::nullopt;
    }

    return ret;
  }

  template <> const Opt<float> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    Opt<float> ret;
    try {
      ret = std::stof(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}" , e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}" , e.what());
      return std::nullopt;
    }

    return ret;
  }
  
  template <> const Opt<double> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    Opt<double> ret;
    try {
      ret = std::stod(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}" , e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}" , e.what());
      return std::nullopt;
    }

    return ret;
  }

  template <> const Opt<std::string> ConfigTable::GetVal(const std::string& section, const std::string& key) const {
    auto ret_str = Get(section , key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}" , key);
      return std::nullopt;
    }

    return ret_str[0];
  }

  std::string ConfigTable::TableString() {
    std::stringstream ss;
    for (auto& [section, keys] : table) {
      ss << "[" << section_map[section] << "]" << std::endl;
      for (auto& [key, value] : keys) {
        if (value.size() == 1) {
          ss << key_map[key] << " = " << value[0] << std::endl;
          continue;
        } else if (value.size() > 1) {
          ss << key_map[key] << " = { " << value[0];
          for (size_t i = 1; i < value.size(); i++) {
            ss << ", " << value[i];
          }
          ss << " } " << std::endl;
        }
      }
    }

    return ss.str();
  }

} // namespace other