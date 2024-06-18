/**
 * \file rendering/render_batch.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_BATCH_HPP
#define OTHER_ENGINE_RENDER_BATCH_HPP

#include <cstdint>
#include <vector>

#include "core/defines.hpp"
#include "rendering/shader.hpp"
#include "rendering/vertex.hpp"
// #include "rendering/texture.hpp"

namespace other {

  struct BatchData {
    uint32_t buffer_cap = 0;
    bool has_indices = false;
    std::vector<uint32_t> buffer_layout = {};
    Path vertex_shader;
    Path fragment_shader;
    Opt<Path> geometry_shader = std::nullopt;
  };
  
  struct Batch {
    bool corrupt = false;
    uint32_t buffer_capacity = 0;
    uint32_t stride = 0;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    bool has_indices = false;

    /// opengl data
    DrawMode draw_mode = DrawMode::TRIANGLES;
    uint32_t renderer_id = 0;
    uint32_t vbo = 0;
    Scope<Shader> shader = nullptr;
    Scope<Shader> texture = nullptr;
    Scope<VertexBuffer> vertex_buffer = nullptr;
    Scope<VertexBuffer> index_buffer = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDER_BATCH_HPP
