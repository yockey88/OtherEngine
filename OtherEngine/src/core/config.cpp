/**
 * \file core/config.cpp
 */
#include "core/config.hpp"

#include <cassert>
#include <string>

#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/uuid.hpp"

#include "rendering/material.hpp"

namespace other {

  void ConfigTable::SetPath(const Path& path) {
    config_path = path;
  }

  Path ConfigTable::GetPath() const {
    return config_path;
  }

  bool ConfigTable::SectionExists(const std::string_view section) const {
    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), ::toupper);

    auto sec_hash = FNV(sec);
    return table.find(sec_hash) != table.end();
  }

  bool ConfigTable::KeyExists(const std::string_view section, const std::string_view key, bool case_sensitive) const {
    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), ::toupper);

    std::string k = key.data();
    if (!case_sensitive) {
      std::transform(k.begin(), k.end(), k.begin(), ::toupper);
    }

    auto sec_hash = FNV(sec);
    auto key_hash = FNV(k);

    auto section_itr = table.find(sec_hash);
    if (section_itr == table.end()) {
      return false;
    }

    const auto& [hash, s] = *section_itr;
    auto key_itr = s.find(key_hash);
    if (key_itr == s.end()) {
      return false;
    }

    return true;
  }

  void ConfigTable::Add(const std::string_view section, const std::string_view key, const std::string_view value, bool is_string, bool allow_key_modifications) {
    if (section.empty()) {
      return;
    }

    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), ::toupper);

    std::string key_str = key.data();
    if (allow_key_modifications) {
      std::transform(key_str.begin(), key_str.end(), key_str.begin(), ::toupper);
    }

    auto sec_hash = FNV(sec);
    auto key_hash = FNV(key_str);

    if (auto section_itr = table.find(sec_hash); section_itr == table.end()) {
      section_map[sec_hash] = section;
      table[sec_hash] = std::map<uint64_t, std::vector<std::string>>();
    }

    if (key_str.empty()) {
      return;
    }

    if (value.empty()) {
      throw IniException(("Value cannot be empty : {}.{}", section, key), IniError::EMPTY_VALUE);
    }

    auto section_itr = table.find(sec_hash);
    assert(section_itr != table.end() && "Section not found in table");
    auto& [hash, s] = *section_itr;

    if (auto key_itr = key_map.find(key_hash); key_itr == key_map.end()) {
      key_map[key_hash] = key_str;

      if (auto key_names_itr = key_names.find(sec_hash); key_names_itr == key_names.end()) {
        key_names[sec_hash] = std::vector<std::string>();
      }

      key_names[sec_hash].push_back(key_str);
    }

    std::string val{ value };
    table[sec_hash][key_hash].push_back(val);
  }

  void ConfigTable::Add(const std::string_view section, const std::string_view key, const std::vector<std::string>& list, bool is_string, bool allow_key_modifications) {
    for (const auto& val : list) {
      Add(section, key, val, is_string, allow_key_modifications);
    }
  }

  void ConfigTable::AddFramebufferSpec(const std::string_view key, const std::string_view value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_framebuffer_specs.find(key_hash) != unparsed_framebuffer_specs.end()) {
      OE_WARN("Duplicate ramebuffer spec! {} already exists", key);
      return;
    }
    unparsed_framebuffer_specs[key_hash] = value;
  }

  void ConfigTable::AddVertexLayout(const std::string_view key, const std::string_view value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_vertex_layouts.find(key_hash) != unparsed_vertex_layouts.end()) {
      OE_WARN("Duplicate vertex layout! {} already exists", key);
      return;
    }
    unparsed_vertex_layouts[key_hash] = value;
  }

  void ConfigTable::AddUniform(const std::string_view key, const std::string_view value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_uniforms.find(key_hash) != unparsed_uniforms.end()) {
      OE_WARN("Duplicate uniform! {} already exists", key);
      return;
    }
    unparsed_uniforms[key_hash] = value;
  }

  void ConfigTable::AddRenderPass(const std::string_view key, const std::string_view value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_render_passes.find(key_hash) != unparsed_render_passes.end()) {
      OE_WARN("Duplicate render pass! {} already exists", key);
      return;
    }
    unparsed_render_passes[key_hash] = value;
  }

  void ConfigTable::AddPipeline(const std::string_view key, const std::string_view value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_pipelines.find(key_hash) != unparsed_pipelines.end()) {
      OE_WARN("Duplicate pipeline! {} already exists", key);
      return;
    }
    unparsed_pipelines[key_hash] = value;
  }

  void ConfigTable::AddScriptSection(const std::string_view key, const UnparsedScriptSection& value) {
    uint64_t key_hash = FNV(key);
    if (unparsed_script_sections.find(key_hash) != unparsed_script_sections.end()) {
      OE_WARN("Duplicate script section! {} already exists", key);
      return;
    }
    unparsed_script_sections[key_hash] = value;
  }

  const std::map<uint64_t, std::vector<std::string>> ConfigTable::Get(const std::string_view section) const {
    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), ::toupper);

    auto sec_hash = FNV(sec);

    if (table.find(sec_hash) != table.end()) {
      return table.at(sec_hash);
    }

    return {};
  }

  const std::vector<std::string> ConfigTable::GetKeys(const std::string_view section) const {
    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), ::toupper);

    uint64_t sec_hash = FNV(sec);
    if (table.find(sec_hash) != table.end()) {
      return key_names.at(sec_hash);
    }

    return {};
  }

  const std::map<uint64_t, std::string>& ConfigTable::GetFramebufferSpecs() const {
    return unparsed_framebuffer_specs;
  }

  const std::map<uint64_t, std::string>& ConfigTable::GetVertexLayouts() const {
    return unparsed_vertex_layouts;
  }

  const std::map<uint64_t, std::string>& ConfigTable::GetUniforms() const {
    return unparsed_uniforms;
  }

  const std::map<uint64_t, std::string>& ConfigTable::GetRenderPasses() const {
    return unparsed_render_passes;
  }

  const std::map<uint64_t, std::string>& ConfigTable::GetPipelines() const {
    return unparsed_pipelines;
  }

  const std::vector<std::string> ConfigTable::Get(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    std::string sec = section.data();
    std::transform(sec.begin(), sec.end(), sec.begin(), toupper);

    std::string k = key.data();
    if (!case_sensitive_key) {
      std::transform(k.begin(), k.end(), k.begin(), toupper);
    }

    auto sec_hash = FNV(sec);
    auto key_hash = FNV(k);

    auto itr = table.find(sec_hash);
    if (itr != table.end()) {
      if (auto itr2 = itr->second.find(key_hash); itr2 != itr->second.end()) {
        std::vector<std::string> ret = itr->second.at(key_hash);
        return ret;
      }

      OE_WARN("Key {} not found in section {}", k, sec);
      return {};
    }

    OE_WARN("Section {} not found", sec);
    return {};
  }

  template <>
  const Opt<glm::vec4> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 4) {
      OE_ERROR("Too many values found for key : {}.{}", section, key);
      return std::nullopt;
    }

    std::vector<float> vals;
    for (const auto& val : ret_str) {
      try {
        vals.push_back(std::stof(val));
      } catch (std::exception& e) {
        OE_ERROR("Invalid value for key : {}", key);
        return std::nullopt;
      }
    }

    OE_ASSERT(vals.size() <= 4, "Too many values found for key : {}", key);

    if (vals.size() == 4) {
      return glm::vec4(vals.at(0), vals.at(1), vals.at(2), vals.at(3));
    } else if (vals.size() == 3) {
      return glm::vec4(vals.at(0), vals.at(1), vals.at(2), 1.f);
    } else if (vals.size() == 1) {
      return glm::vec4(vals.at(0), 0.f, 0.f, 1.f);
    }

    OE_ERROR("Invalid value for key : {}", key);
    return std::nullopt;
  }

  template <>
  const Opt<bool> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    if (ret_str.at(0) == "TRUE" || ret_str.at(0) == "true") {
      return true;
    } else if (ret_str.at(0) == "FALSE" || ret_str.at(0) == "false") {
      return false;
    }

    OE_ERROR("Invalid value for key : {}", key);
    return std::nullopt;
  }

  template <>
  const Opt<uint8_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<uint16_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<uint32_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<uint64_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoul(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key {} : {}", key, ret_str.at(0));
      return std::nullopt;
    }
  }

  template <>
  const Opt<UUID> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return Opt<UUID>(GetVal<uint64_t>(section, key, case_sensitive_key));
  }

  template <>
  const Opt<int8_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<int16_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<int32_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    try {
      return std::stoi(ret_str.at(0));
    } catch (std::exception& e) {
      OE_ERROR("Invalid value for key : {}", key);
      return std::nullopt;
    }
  }

  template <>
  const Opt<int64_t> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    Opt<int64_t> ret;
    try {
      ret = std::stoll(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}", e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}", e.what());
      return std::nullopt;
    }

    return ret;
  }

  template <>
  const Opt<float> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    Opt<float> ret;
    try {
      ret = std::stof(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}", e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}", e.what());
      return std::nullopt;
    }

    return ret;
  }

  template <>
  const Opt<double> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    Opt<double> ret;
    try {
      ret = std::stod(ret_str[0]);
    } catch (std::invalid_argument& e) {
      OE_ERROR("Invalid argument : {}", e.what());
      return std::nullopt;
    } catch (std::out_of_range& e) {
      OE_ERROR("Out of range : {}", e.what());
      return std::nullopt;
    }

    return ret;
  }

  template <>
  const Opt<std::string> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    auto ret_str = Get(section, key, case_sensitive_key);
    if (ret_str.empty()) {
      return std::nullopt;
    }

    if (ret_str.size() > 1) {
      OE_ERROR("More than one value found for key : {}", key);
      return std::nullopt;
    }

    return ret_str[0];
  }

  namespace {

    template <typename T>
    const Opt<std::vector<T>> GetListVal(const std::string_view section, const std::string_view key, const ConfigTable& table, bool case_sensitive_key) {
      auto ret_str = table.Get(section, key, case_sensitive_key);
      if (ret_str.empty()) {
        return std::nullopt;
      }

      std::vector<T> ret;
      for (const auto& val : ret_str) {
        try {
          if constexpr (std::is_same_v<T, int8_t>) {
            ret.push_back(std::stoi(val));
          } else if constexpr (std::is_same_v<T, uint8_t>) {
            ret.push_back(std::stoul(val));
          } else if constexpr (std::is_same_v<T, int16_t>) {
            ret.push_back(std::stoi(val));
          } else if constexpr (std::is_same_v<T, uint16_t>) {
            ret.push_back(std::stoul(val));
          } else if constexpr (std::is_same_v<T, int32_t>) {
            ret.push_back(std::stoi(val));
          } else if constexpr (std::is_same_v<T, uint32_t>) {
            ret.push_back(std::stoul(val));
          } else if constexpr (std::is_same_v<T, int64_t>) {
            ret.push_back(std::stoll(val));
          } else if constexpr (std::is_same_v<T, uint64_t>) {
            ret.push_back(std::stoull(val));
          } else if constexpr (std::is_same_v<T, float>) {
            ret.push_back(std::stof(val));
          } else if constexpr (std::is_same_v<T, double>) {
            ret.push_back(std::stod(val));
          }
        } catch (std::exception& e) {
          OE_ERROR("Invalid value for key : {}", key);
          return std::nullopt;
        }
      }

      return ret;
    }

  }  // anonymous namespace

  template <>
  const Opt<std::vector<int8_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<int8_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<uint8_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<uint8_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<int16_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<int16_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<uint16_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<uint16_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<int32_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<int32_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<uint32_t>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<uint32_t>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<float>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<float>(section, key, *this, case_sensitive_key);
  }

  template <>
  const Opt<std::vector<double>> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
    return GetListVal<double>(section, key, *this, case_sensitive_key);
  }

  // template <>
  // const Opt<Material> ConfigTable::GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key) const {
  //   auto color = GetVal<glm::vec4>(section, std::string{ key } + ".COLOR", case_sensitive_key);
  //   auto shininess = GetVal<float>(section, std::string{ key } + ".SHININESS", case_sensitive_key);

  //   if (!color.has_value() && !shininess.has_value()) {
  //     return std::nullopt;
  //   }

  //   return Material(color.value_or(glm::vec4(1.f)), shininess.value_or(32.f));
  // }

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

}  // namespace other
