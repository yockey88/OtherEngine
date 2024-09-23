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

/** Idea of render-pass chaining for dynamic pipeline construction
 *
 *  - implement a chaining utility to dynamically construct render pass chains 
 *      to allow for custom pipelines 
 *
 *  - let E = scene-lighting-environment
 *      accessible globally
 *
 *  - renderpass rp : X -> Y 
 *      where X is either 
 *       1 - a scene mesh/uniform set
 *       2 - a set of render buffers/textures
 *    ex:
 *      let g_rp = gbuffer_pass 
 *      then 
 *        X = { geometry-data , transforms , materials }
 *        Y_g = { position , normal , albedo , depth }
 *
 *      let l_rp = lighting_pass
 *      then 
 *        X = Y_g U { textured-quad-mesh } 
 *        Y_l = { lit-scene-texture }
 *      
 *      so that this hardcoded pipeline below cane be thought of as
 *        output-frame = l_rp o g_rp = l_rp(g_rp(X))
 *
 *  - a generic pipeline then is 
 *      pl : P -> i,
 *        P is a subset of Rp (the set of all renderpasses),
 *          P = { p_0 , ... , p_n }
 *        i \in I (the set of output images of the engine in a single run)
 *
 *      pl : p_n o ... o p_0 = p_n(...p_1(p_0(G))) = i;
 *        G = { geometry-data , transforms , materials }
 **/

/** Pseudocode for above idea
 *
 *  - class Pipeline(pass-list) {
 *      P <- pass-list
 *    }
 *
 *    Image Pipeline::Render(scene-description) {
 *      G = { 
 *        scene-description.geometry ,
 *        scene-description.transforms ,
 *        scene-description.materials ,
 *      }
 *
 *      return execute_chain(G);
 *    }
 *
 *    Image Pipeline::execute_chain(geometry) {
 *      return fold_left(P , geometry , (G , func) { return func(G) });
 *    }
 *
 **/

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
