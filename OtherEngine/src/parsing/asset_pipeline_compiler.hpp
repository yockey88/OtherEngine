/**
 * \file parsing/asset_pipeline_compiler.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_PIPELINE_COMPILER_HPP
#define OTHER_ENGINE_ASSET_PIPELINE_COMPILER_HPP

#include <string_view>

#include "asset/asset_pipeline.hpp"

namespace other {

  class AssetPipelineCompiler {
   public:
    AssetPipelineCompiler() = default;
    ~AssetPipelineCompiler() = default;

    AssetPipeline CompileSource(const std::string_view src);

   private:
    size_t index = 0;
    std::string source;
    std::vector<std::string> tokens;

    AssetPipeline::Command ParseCommand(const std::string_view line);
    uint64_t ParseAction(std::vector<std::string>& action_tokens, const std::vector<std::string>& tokens);

    AssetPipeline pipeline;

    bool AtEnd() const;
    char Peek() const;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_PIPELINE_COMPILER_HPP