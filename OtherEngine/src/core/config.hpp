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

    void SetPath(const Path& path);
    Path GetPath() const;

    bool SectionExists(const std::string_view section) const;
    bool KeyExists(const std::string_view section, const std::string_view key, bool case_sensitive = false) const;

    void Add(const std::string_view section, const std::string_view key = "", const std::string_view value = "", bool is_string = false, bool allow_key_modifications = true);
    void Add(const std::string_view section, const std::string_view key, const std::vector<std::string>& list, bool is_string = false, bool allow_key_modifications = true);

    void AddFramebufferSpec(const std::string_view key, const std::string_view value);
    void AddVertexLayout(const std::string_view key, const std::string_view value);
    void AddUniform(const std::string_view key, const std::string_view value);
    void AddRenderPass(const std::string_view key, const std::string_view value);
    void AddPipeline(const std::string_view key, const std::string_view value);

    struct UnparsedScriptSection {
      ValueType type;
      std::string source;
    };

    void AddScriptSection(const std::string_view key, const UnparsedScriptSection& value);

    const std::map<uint64_t, std::vector<std::string>> Get(const std::string_view section) const;
    const std::vector<std::string> Get(const std::string_view section, const std::string_view key, bool case_sensitive_key = false) const;
    const std::vector<std::string> GetKeys(const std::string_view section) const;

    const std::map<uint64_t, std::string>& GetFramebufferSpecs() const;
    const std::map<uint64_t, std::string>& GetVertexLayouts() const;
    const std::map<uint64_t, std::string>& GetUniforms() const;
    const std::map<uint64_t, std::string>& GetRenderPasses() const;
    const std::map<uint64_t, std::string>& GetPipelines() const;

    template <typename T>
    const Opt<T> GetVal(const std::string_view section, const std::string_view key, bool case_sensitive_key = false) const;

    std::string TableString() const;

   private:
    Path config_path;
    /// section hash -> section name
    std::map<uint64_t, std::string> section_map;
    /// key hash -> key name
    std::map<uint64_t, std::string> key_map;
    /// section hash -> list of key names
    std::map<uint64_t, std::vector<std::string>> key_names;
    /// section hash -> (key hash -> list of raw str values)
    std::map<uint64_t, std::map<uint64_t, std::vector<std::string>>> table;

    std::map<uint64_t, std::string> unparsed_framebuffer_specs;
    std::map<uint64_t, std::string> unparsed_vertex_layouts;
    std::map<uint64_t, std::string> unparsed_uniforms;
    std::map<uint64_t, std::string> unparsed_render_passes;
    std::map<uint64_t, std::string> unparsed_pipelines;

    std::map<uint64_t, UnparsedScriptSection> unparsed_script_sections;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_CONFIG_HPP
