/**
 * \file rendering\renderer.cpp
 */
#include "rendering\renderer.hpp"

#include <glad/glad.h>

#include "rendering/rendering_defines.hpp"

namespace other {

  RenderData Renderer::render_data;

  void Renderer::Initialize(const ConfigTable& config) {
    auto win_cfg = Window::ConfigureWindow(config);
    auto win_res = Window::GetWindow(win_cfg, config);

    CHECKGL();

    if (win_res.IsErr()) {
      throw std::runtime_error("Failed to create engine window!"); 
    }

    render_data.window = std::move(win_res.Unwrap());

    FramebufferSpec fb_spec {
      .clear_color = render_data.window->ClearColor() ,
      .size = { WindowSize().x , WindowSize().y } ,
    };
    render_data.frame = Ref<Framebuffer>::Create(fb_spec);

    CHECKGL();
  }
      
  void Renderer::SetSceneContext(const Ref<Scene>& scene) {
  }
  
  void Renderer::BindCamera(Ref<CameraBase>& camera) {
    render_data.active_camera = camera;
  }

  void Renderer::BeginFrame() {
    render_data.window->Clear();
    render_data.frame->BindFrame();

    CHECKGL();
  }
  
#if 0
  void Renderer::DrawLine(const Point& start , const Point& end , const RgbColor& color) {
    Vertex s , e;
    s.position = start;
    s.color = color;

    e.position = end;
    e.color = color;
    
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , s);
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , e);
  }

  void Renderer::DrawLine(const Vertex& start , const Vertex& end) {
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , start);
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , end);
  }
  
  void Renderer::DrawLine(const Line& line , const RgbColor& color) {
    Vertex s , e;
    s.position = line.start;
    s.color = color;

    e.position = line.end;
    e.color = color;
    
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , s);
    WriteVertexToBatch(render_data.primitive_batches[kLineBatchIndex] , e);
  }
  
  void Renderer::DrawTriangle(const Point& p1 , const Point& p2 , const Point& p3 , const RgbColor& color) {
    Vertex v1 , v2 , v3;
    v1.position = p1;
    v1.color = color;

    v2.position = p2;
    v2.color = color;

    v3.position = p3;
    v3.color = color;
    
    WriteVertexToBatch(render_data.primitive_batches[kTriangleBatchIndex] , v1);
    WriteVertexToBatch(render_data.primitive_batches[kTriangleBatchIndex] , v2);
    WriteVertexToBatch(render_data.primitive_batches[kTriangleBatchIndex] , v3);
  }

  void Renderer::DrawTriangle(const std::array<Vertex , 3>& corners) {
    for (const auto& v : corners) {
      WriteVertexToBatch(render_data.primitive_batches[kTriangleBatchIndex] , v);
    }
  }

  void Renderer::DrawTriangle(const Triangle& triangle , const RgbColor& color) {
    DrawTriangle(triangle.p1 , triangle.p2 , triangle.p3 , color);
  }

  void Renderer::DrawRect(const Point& position , const glm::vec2& half_extents , const RgbColor& color) {
    Vertex v1 , v2 , v3 , v4;
    v1.position = {
      position.x + half_extents.x ,
      position.y + half_extents.y ,
      position.z
    };
    v1.color = color;

    v2.position = {
      position.x - half_extents.x ,
      position.y + half_extents.y ,
      position.z
    };
    v2.color = color;

    v3.position = {
      position.x - half_extents.x ,
      position.y - half_extents.y ,
      position.z
    };
    v3.color = color;

    v4.position = {
      position.x + half_extents.x ,
      position.y - half_extents.y ,
      position.z
    };
    v4.color = color;

    /// TODO: this works but id rather use indices to avoid duplicate vertices :/
    ///       - find out why index buffer crashes

    // WriteRectIndices();
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v1); // 0
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v2); // 1
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v4); // 3
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v2); // 1
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v3); // 2
    WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v4); // 3
  }

  void Renderer::DrawRect(const std::span<Vertex , 4>& corners) {
    // WriteRectIndices(); 
    for (const auto& v : corners) {
      WriteVertexToBatch(render_data.primitive_batches[kRectBatchIndex] , v);
    }
  }

  void Renderer::DrawRect(const Rect& rect , const RgbColor& color) {
    DrawRect(rect.position , rect.half_extents , color);    
  }
      
  void Renderer::DrawStaticModel(Ref<StaticModel> cube_model) {
     
  }

#endif

  void Renderer::EndFrame() {
    render_data.frame->UnbindFrame();
  }
      
  void Renderer::SwapBuffers() {
    render_data.window->SwapBuffers();
  }

  glm::ivec2 Renderer::WindowSize() {
    return render_data.window->Size();
  }

  void Renderer::HandleWindowResize(const glm::ivec2& size) {
    render_data.window->Resize(size);
  }
      
  bool Renderer::IsWindowFocused() {
    return render_data.window->HasFocus();
  }

  void Renderer::Shutdown() {
    DestroyBatches();
    DestroyPrimitiveBatches();
    
    CHECKGL();
    
    render_data.window = nullptr;
  }
  
  const Scope<Window>& Renderer::GetWindow() { 
    return render_data.window; 
  }
      
  Ref<Framebuffer>& Renderer::Viewport() {
    return render_data.frame;
  }

  void Renderer::ClearColor(const std::vector<std::string>& color) {
    glm::vec4 c;
    for (int i = 0; i < color.size(); i++) {
      c[i] = std::stof(color[i]);
    }
    render_data.window->SetClearColor(c);
  }
      
  void Renderer::ClearColor(const glm::vec4& color) { 
    render_data.window->SetClearColor(color); 
  }

#if 0
  void Renderer::WriteVertexToBatch(Batch& batch , const Vertex& vert) {
    batch.vertices.push_back(vert.position.x);
    batch.vertices.push_back(vert.position.y);
    batch.vertices.push_back(vert.position.z);
    
    batch.vertices.push_back(vert.color.x);
    batch.vertices.push_back(vert.color.y);
    batch.vertices.push_back(vert.color.z);

    if (vert.normal.has_value()) {
      batch.vertices.push_back(vert.normal.value().x);
      batch.vertices.push_back(vert.normal.value().y);
      batch.vertices.push_back(vert.normal.value().z);
    } else {
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
    }
    
    if (vert.tangent.has_value()) {
      batch.vertices.push_back(vert.tangent.value().x);
      batch.vertices.push_back(vert.tangent.value().y);
      batch.vertices.push_back(vert.tangent.value().z);
    } else {
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
    }
    
    if (vert.bitangent.has_value()) {
      batch.vertices.push_back(vert.bitangent.value().x);
      batch.vertices.push_back(vert.bitangent.value().y);
      batch.vertices.push_back(vert.bitangent.value().z);
    } else {
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
    }
    
    if (vert.uv_coord.has_value()) {
      batch.vertices.push_back(vert.uv_coord.value().x);
      batch.vertices.push_back(vert.uv_coord.value().y);
    } else {
      batch.vertices.push_back(0);
      batch.vertices.push_back(0);
    }
  }
#endif 

  void Renderer::WriteRectIndices() {
  }

#if 0
  void Renderer::RenderBatch(Batch& batch) {
    /// display some sort of text to screen about no camera to use
    // if (render_data.active_camera == nullptr) {
    //   return;
    // }
    // 
    // if (batch.corrupt || batch.vertices.size() == 0) {
    //   return;
    // }


    // if (batch.shader == nullptr || batch.vertex_buffer == nullptr || 
    //     (batch.has_indices && batch.index_buffer == nullptr)) { 
    //   batch.corrupt = true;
    //   return;
    // } 
    // 
    // const glm::mat4& camera_matrix = render_data.active_camera->GetMatrix();

    // batch.shader->Bind();
    // batch.shader->SetUniform("ucamera" , camera_matrix);
    // 
    // batch.vertex_buffer->Bind();
    // batch.vertex_buffer->BufferData(batch.vertices.data() , batch.vertices.size() * sizeof(float));
    // 
    // /// 
    // // if (batch.has_indices && batch.index_buffer != nullptr) {
    // //   batch.index_buffer->Bind();
    // //   batch.index_buffer->BufferData(batch.indices.data() , batch.indices.size() * sizeof(uint32_t));
    // // }

    // glBindVertexArray(batch.renderer_id);
    // if (batch.has_indices) {
    //   // glDrawElements(batch.draw_mode , batch.indices.size() , GL_UNSIGNED_INT , 0);
    // } else {
    //   glDrawArrays(batch.draw_mode , 0 , batch.vertices.size() / batch.stride);
    // }

    // batch.vertex_buffer->ClearBuffer();
    // batch.vertices.clear();
    // batch.indices.clear();
    // 
    // if (batch.index_buffer != nullptr) {
    //   batch.index_buffer->ClearBuffer();
    // }
  }

  void Renderer::CreateRenderBatch(const BatchData& data , uint32_t idx , bool internal) {
    auto& batch = internal ? 
      render_data.primitive_batches[idx] : 
      render_data.scene_batches[idx];

    glGenVertexArrays(1 , &batch.renderer_id);
    glBindVertexArray(batch.renderer_id);

    /// we use the same buffer cap for each for simplicity even though the index buffer techniically needs less
    batch.vertex_buffer = NewScope<VertexBuffer>(BufferType::ARRAY_BUFFER , data.buffer_cap);
    
    batch.has_indices = data.has_indices;
    if (batch.has_indices) {
      batch.index_buffer = NewScope<VertexBuffer>(BufferType::ELEMENT_ARRAY_BUFFER , data.buffer_cap);
    }


    for (const auto& attr : data.buffer_layout) {
      batch.stride += attr;
    }

    uint32_t attr_count = 0;
    uint32_t offset = 0;
    for (const auto& attr : data.buffer_layout) {
      glEnableVertexAttribArray(attr_count); 
      glVertexAttribPointer(attr_count , attr , GL_FLOAT , GL_FALSE , batch.stride * sizeof(float) , (void*)(offset * sizeof(float)));

      attr_count++;
      offset += attr;
    }
  }
      
  Batch Renderer::CreateRenderBatch(const BatchData& data) {

    return Batch{}; 
  }
#endif

  void Renderer::CreatePrimitiveBatches() {
  }

  void Renderer::DestroyPrimitiveBatches() {
  }
      
  void Renderer::DestroyBatches() {
  }

} // namespace other
