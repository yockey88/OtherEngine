/**
 * \file rendering/render_pass.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_PASS_HPP
#define OTHER_ENGINE_RENDER_PASS_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/buffer.hpp"

#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"
#include "rendering/point_light.hpp"

namespace other {

  struct RenderPassSpec {
    std::string name = "";
    glm::vec4 tag_col;

    std::vector<Uniform> uniforms;
    Ref<Shader> shader = nullptr;
  };

  class RenderPass : public RefCounted {
    public:
      RenderPass(RenderPassSpec spec);
      virtual ~RenderPass() override {}

      std::string Name() const;

      void Bind();
      Ref<Shader> GetShader();
      void Unbind();

      void DefineInput(const Ref<UniformBuffer>& uniform_block);
      void DefineInput(Uniform uniform);
      
      template <typename T>
      void SetInput(const std::string_view block_name , const std::string_view name , T val , uint32_t index = 0) {
        uint64_t blck_hash = FNV(block_name);

        auto itr = uniform_blocks.find(blck_hash);
        if (itr == uniform_blocks.end()) {
          OE_ERROR("Failed to set uniform {}, block {} not defined in pass {}" , name , block_name , spec.name);
          return;
        }

        auto& [id , uni_blck] = *itr;
        uni_blck->SetUniform(name , val , index);
      }

      /// FIXME: do something about this mess
      template <typename T>
      void SetInput(const std::string_view name , T val , uint32_t index = 0) {
        uint64_t hash = FNV(name);

        auto itr = uniforms.find(hash);
        if (itr == uniforms.end()) {
          OE_ERROR("Failed to set uniform {}, not defined in render pass {}" , name , spec.name);
          return;
        }

        auto& [id , uni] = *itr;
        if (sizeof(T) != GetValueSize(uni.type)) {
          OE_ERROR("Attempting to set uniform {} to invalidly sized type {}" , name , typeid(T).name());
          return;
        }

        if constexpr (std::same_as<T , int32_t>) {
          auto itr = int_processors.find(id);
          if (itr != int_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , float>) {
          auto itr = float_processors.find(id);
          if (itr != float_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::vec2>) {
          auto itr = vec2_processors.find(id);
          if (itr != vec2_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::vec3>) {
          auto itr = vec3_processors.find(id);
          if (itr != vec3_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::vec4>) {
          auto itr = vec4_processors.find(id);
          if (itr != vec4_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::mat2>) {
          auto itr = mat2_processors.find(id);
          if (itr != mat2_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::mat3>) {
          auto itr = mat3_processors.find(id);
          if (itr != mat3_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        } else if constexpr (std::same_as<T , glm::mat4>) {
          auto itr = mat4_processors.find(id);
          if (itr != mat4_processors.end()) {
            auto& [id2 , processor] = *itr;
            processor(val);
          }
        }

        spec.shader->SetUniform(uni.name , val , index);
      }

      virtual void SetRenderState() {}
      /// just return them by default
      virtual Buffer ProcessModels(Buffer& buffer) { return buffer; }
      virtual Buffer ProcessMaterials(Buffer& buffer) { return buffer; }

      template <typename T>
      using UniformProcessor = std::function<void(T&)>;

      template <typename T>
      void RegisterUniformProcessor(const std::string_view name , UniformProcessor<T> processor) {
        UUID hash = FNV(name);

        if constexpr (std::same_as<T , int32_t>) {
          int_processors[hash] = processor;
        } else if constexpr (std::same_as<T , float>) {
          vec4_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::vec2>) {
          vec4_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::vec3>) {
          vec3_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::vec4>) {
          vec4_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::mat2>) {
          mat2_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::mat3>) {
          mat3_processors[hash] = processor;
        } else if constexpr (std::same_as<T , glm::mat4>) {
          mat4_processors[hash] = processor;
        }
      }

    private:
      std::map<UUID , Uniform> uniforms;
      std::map<UUID , Ref<UniformBuffer>> uniform_blocks;

      RenderPassSpec spec;

      template <typename T>
      using UniformProcessorMap = std::unordered_map<UUID , UniformProcessor<T>>;

      UniformProcessorMap<int32_t> int_processors;
      UniformProcessorMap<float> float_processors;
      UniformProcessorMap<glm::vec2> vec2_processors;
      UniformProcessorMap<glm::vec3> vec3_processors;
      UniformProcessorMap<glm::vec4> vec4_processors;
      UniformProcessorMap<glm::mat2> mat2_processors;
      UniformProcessorMap<glm::mat3> mat3_processors;
      UniformProcessorMap<glm::mat4> mat4_processors;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RENDER_PASS_HPP
