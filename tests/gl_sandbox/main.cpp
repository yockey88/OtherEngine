/**
 * \file gl_sandbox/main.cpp
 **/
#include <iostream>

#include <SDL_events.h>
#include <SDL_video.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <box2d/b2_world.h>
#include <box2d/box2d.h>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>
#include <rendering/gbuffer.hpp>

#include "core/defines.hpp"
#include "core/errors.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"

#include "physics/physics_engine.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/direction_light.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/material.hpp"
#include "rendering/material_table.hpp"
#include "rendering/model.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/point_light.hpp"
#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/shader.hpp"
#include "rendering/ui/ui.hpp"
#include "rendering/uniform.hpp"
#include "rendering/vertex.hpp"
#include "rendering/window.hpp"
#include "scripting/script_engine.hpp"

#include "gl_helpers.hpp"
#include "sandbox_ui.hpp"
#include "shader_embed.hpp"

using namespace other;

struct Shape {
  uint32_t vao = 0, vbo = 0, ebo = 0;
  std::vector<SubMesh> submeshes;
  std::vector<MeshNode> nodes;

  BBox mesh_bounds = BBox::empty;

  std::vector<Vertex> vertices;
  std::vector<Index> indices;

  std::vector<float> raw_vertices;
  std::vector<uint32_t> raw_indices;
  std::vector<uint32_t> raw_layout;

  Ref<VertexArray> mesh_vao;

  Shape(const Path& path, Ref<MaterialTable>& material_table);
  void Draw(DrawMode mode);

 private:
  std::vector<other::UUID> material_ids;
  std::map<uint32_t, other::UUID> submesh_materials;

  void ProcessNode(const aiNode* node, const aiScene* scene);
  void ProcessMesh(const aiMesh* mesh, const aiScene* scene);
  void TraverseNodes(const aiNode* node, int32_t node_idx, const glm::mat4& parent_transform = glm::mat4(1.f), uint32_t level = 0);
  void GetMaterials(const aiScene* scene);
};

struct Quad {
  uint32_t vao = 0, vbo = 0, ebo = 0;
  std::vector<float> vertices;
  // clang-format off
  std::vector<uint32_t> indices = {
    0, 1, 2,
    1, 2, 3,
  };

  Quad();
  ~Quad();

  void Draw();
};
// clang-format on

struct Cube {
  uint32_t vao = 0, vbo = 0, ebo = 0;
  std::vector<float> vertices;
  // clang-format off
  std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
  };
  // clang-format on

  Cube();
  ~Cube();

  void Draw(other::DrawMode mode, uint32_t instances = 1);
};

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

void UpdateCamera(other::Ref<CameraBase>& camera);

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  other::CmdLine cmd_line(__argc, __argv);
#else
int main(int argc, char* argv[]) {
  other::CmdLine cmd_line(argc, argv);
#endif
  int exit = 0;
  try {
    const other::Path glsandbox_dir = "C:/Yock/code/OtherEngine/tests/gl_sandbox";
    const other::Path config_path = glsandbox_dir / "gl_sandbox.other";

    cmd_line.SetFlag("--project", { config_path.string() });

    other::Engine mock_engine(cmd_line, "Sandbox--Thread");

    AppState::Initialize(&mock_engine);
    Renderer::Initialize(mock_engine.config);
    UI::Initialize(mock_engine.config, Renderer::GetWindow());
    ScriptEngine::Initialize(mock_engine.config);
    PhysicsEngine::Initialize(mock_engine.config);
    AppState::AttachApplication();
    AppState::mode = EngineMode::EDITOR;

    uint32_t shader1 = other::GetShader(vert1, frag1);
    uint32_t shader2 = other::GetShader(vert2, frag2);

    Ref<Framebuffer> frame = NewRef<Framebuffer>(other::FramebufferSpec{});

    std::vector<float> fb_verts = {
      1.f, 1.f, 1.f, 1.f,
      -1.f, 1.f, 0.f, 1.f,
      -1.f, -1.f, 0.f, 0.f,
      1.f, -1.f, 1.f, 0.f
    };

    std::vector<uint32_t> fb_indices = {
      0, 1, 3,
      1, 2, 3
    };
    std::vector<uint32_t> fb_layout = {
      2, 2
    };

    Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts, fb_indices, fb_layout);

    const other::Path shader_dir = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const other::Path fbshader_path = shader_dir / "fbshader.oshader";
    const other::Path default_shader_path = shader_dir / "default.oshader";
    const other::Path gbuffer_shader_path = shader_dir / "gbuffer.oshader";
    Ref<Shader> fb_shader = other::BuildShader(fbshader_path);
    Ref<Shader> default_shader = other::BuildShader(default_shader_path);
    Ref<Shader> gbuffer_shader = other::BuildShader(gbuffer_shader_path);

    // const other::Path deferred_shader_path = shader_dir / "deferred_shading.oshader";
    // Ref<Shader> deferred_shader = other::BuildShader(deferred_shader_path);
    // deferred_shader->Bind();
    // deferred_shader->SetUniform("goe_position", 0);
    // deferred_shader->SetUniform("goe_normal", 1);
    // deferred_shader->SetUniform("goe_albedo", 2);
    // deferred_shader->Unbind();

    const other::Path gl_sb_material_dir = other::Filesystem::GetEngineCoreDir() / "tests" / "gl_sandbox" / "shaders";
    const other::Path mat_path = gl_sb_material_dir / "mat.oshader";

    Ref<Shader> mat_shader = other::BuildShader(mat_path);
    // Ref<Shader> mat2_shader = other::BuildShader(mat2_path);

    other::Ref<CameraBase> camera = other::NewRef<PerspectiveCamera>(glm::ivec2{ win_w, win_h });
    camera->SetPosition({ 0.f, 0.f, 3.f });
    glm::mat4 proj = camera->ProjectionMatrix();
    glm::mat4 view = camera->ViewMatrix();

    glm::mat4 model1 = glm::mat4(1.0f);
    float m1_rotation = 0.f;

    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(2.f, 0.f, 0.f));

    glm::mat4 model3 = glm::mat4(1.0f);
    model3 = glm::translate(model3, glm::vec3(-2.f, 0.f, 0.f));

    other::PointLight point_light{
      .position = { 1.2f, 1.0f, 2.0f, 1.f },
      .color = { 0.2f, 0.2f, 0.2f, 1.f },
    };
    other::DirectionLight dir_light{
      .direction = { -0.2f, 1.0f, -0.3f, 1.f },
      .color = { 1.0f, 1.0f, 1.0f, 1.f },
    };

    CHECKGL();

    uint32_t camera_binding_pnt = 0;
    std::vector<other::Uniform> camera_unis = {
      { "projection", other::ValueType::MAT4 },
      { "view", other::ValueType::MAT4 },
      { "viewpoint", other::ValueType::VEC4 },
    };

    uint32_t model_binding_pnt = 1;
    std::vector<other::Uniform> model_unis = {
      { "models", other::ValueType::MAT4, 100 },
    };

    uint32_t material_binding_pnt = 2;
    std::vector<other::Uniform> material_unis = {
      { "materials", other::ValueType::USER_TYPE, 100, sizeof(other::Material) },
    };

    uint32_t light_binding_pnt = 3;
    std::vector<other::Uniform> light_unis = {
      { "num_lights", other::ValueType::VEC4 },
      { "point_lights", other::ValueType::USER_TYPE, 100, sizeof(other::PointLight) },
      { "direction_lights", other::ValueType::USER_TYPE, 100, sizeof(other::DirectionLight) },
    };

    Ref<other::UniformBuffer> camera_uniforms = NewRef<other::UniformBuffer>("Camera", camera_unis, camera_binding_pnt);
    camera_uniforms->BindBase();
    Ref<other::UniformBuffer> light_uniforms = NewRef<other::UniformBuffer>("Lights", light_unis, light_binding_pnt, other::SHADER_STORAGE);
    light_uniforms->BindBase();

    Ref<other::UniformBuffer> material_uniforms = NewRef<other::UniformBuffer>("MaterialData", material_unis, material_binding_pnt, other::SHADER_STORAGE);
    material_uniforms->BindBase();
    Ref<other::UniformBuffer> model_uniforms = NewRef<other::UniformBuffer>("ModelData", model_unis, model_binding_pnt, other::SHADER_STORAGE);
    model_uniforms->BindBase();

    other::Buffer model_buffer;
    other::Buffer material_buffer;

    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2, "g_position"), 0);
    glUniform1i(glGetUniformLocation(shader2, "g_normal"), 1);
    glUniform1i(glGetUniformLocation(shader2, "g_albedo_spec"), 2);
    glUseProgram(0);

    /// generate sampler2DArray
    std::vector<glm::vec4> colors = {
      { 1.f, 0.f, 0.f, 1.f },
      { 0.f, 1.f, 0.f, 1.f },
    };

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    other::UUID mat1 = material_table->RegisterMaterial(colors[0], glm::vec4(1.f), glm::vec4(1.f));
    other::UUID mat2 = material_table->RegisterMaterial(colors[1], glm::vec4(1.f), glm::vec4(1.f));

    OE_DEBUG("Uniforms Set");

    other::GBuffer gbuffer({ win_w, win_h });

    OE_INFO("Running");

    Quad quad;
    Cube cube;

    Shape shape(glsandbox_dir / "assets" / "backpack.fbx", material_table);

    Ref<Directory> dir = Filesystem::GetDirectory("assets");
    if (dir == nullptr) {
      OE_ERROR("Failed to get directory");
      throw std::runtime_error("Failed to get directory");
    }
    Ref<FileHandle> file = dir->GetFile("backpack.fbx");
    if (file == nullptr) {
      OE_ERROR("Failed to get file");
      throw std::runtime_error("Failed to get file");
    }

    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(file->handle, file->GetAssetType());
    if (source == nullptr) {
      OE_ERROR("Failed to get model source");
      throw std::runtime_error("Failed to get model source");
    }

    CHECKGL();

    bool running = true;

    bool camera_lock = true;
    bool force_update = true;
    other::Mouse::FreeCursor();

    while (running) {
      other::IO::Update();

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT: running = false; break;
          case SDL_WINDOWEVENT:
            switch (event.window.event) {
              case SDL_WINDOWEVENT_CLOSE: running = false; break;
              default:
                break;
            }
            break;
          case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE:
                running = false;
                break;
                break;
              case SDLK_c:
                camera_lock = !camera_lock;
                if (camera_lock) {
                  other::Mouse::FreeCursor();
                } else {
                  other::Mouse::LockCursor();
                }
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);
      }
      if (!running) {
        break;
      }

      other::EventQueue::Clear();

      other::Renderer::GetWindow()->Clear();

      /// update camera uniforms
      if (!camera_lock || force_update) {
        force_update = !force_update;
        UpdateCamera(camera);
        proj = camera->ProjectionMatrix();
        view = camera->ViewMatrix();
        glm::vec4 cam_pos = glm::vec4(camera->Position(), 1.f);

        camera_uniforms->SetUniform("projection", camera->ProjectionMatrix());
        camera_uniforms->SetUniform("view", camera->ViewMatrix());
        camera_uniforms->SetUniform("viewpoint", cam_pos);
      }

      light_uniforms->SetUniform("num_lights", glm::vec4{ 0, 1, 0, 0 });
      light_uniforms->SetUniform("point_lights", point_light);
      light_uniforms->SetUniform("direction_lights", dir_light);

      model1 = glm::mat4(1.f);
      model1 = glm::rotate(model1, m1_rotation, { 1.f, 1.f, 1.f });
      m1_rotation += 0.1f;

      ///> GBUFFER RENDER
      /// start material textures at the the end of gbuffer textures
      // material_table->Bind(GBuffer::NUM_TEX_IDXS);
      // gbuffer.SetInput("albedo_textures", GBuffer::NUM_TEX_IDXS);
      // gbuffer.Bind();
      // cube.Draw(other::TRIANGLES, 2);
      // gbuffer.Unbind();
      // material_table->Unbind();
      /// > GBUFFER RENDER

      /// > LIGHTING PASS
      // frame->BindFrame();
      // deferred_shader->Bind();
      // fb_mesh->Draw(other::TRIANGLES);
      // deferred_shader->Unbind();
      // frame->UnbindFrame();
      /// > LIGHTING PASS

      // /// > GEOMETRY PASS
      frame->BindFrame();
      material_table->Bind();
      mat_shader->Bind();
      mat_shader->SetUniform("albedo_textures", 0);
      mat_shader->SetUniform("normal_textures", 1);
      mat_shader->SetUniform("roughness_textures", 2);

      // default_shader->Bind();
      // default_shader->SetUniform("albedo_textures", 0);
      // default_shader->SetUniform("normal_textures", 1);
      // default_shader->SetUniform("roughness_textures", 2);

      model_buffer.ZeroMem();
      model_buffer.BufferData(model1);
      model_buffer.BufferData(model2);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      MaterialTable::Material mat1_data = material_table->GetMaterial(mat1);
      MaterialTable::Material mat2_data = material_table->GetMaterial(mat2);
      Material gpumat1 = mat1_data;
      Material gpumat2 = mat2_data;

      material_buffer.ZeroMem();
      material_buffer.BufferData(gpumat1);
      material_buffer.BufferData(gpumat2);

      material_uniforms->BindBase();
      material_uniforms->LoadFromBuffer(material_buffer);

      cube.Draw(other::TRIANGLES, colors.size());

      model_buffer.ZeroMem();
      model_buffer.BufferData(model3);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      // MaterialTable::Material mat1_data = material_table->GetMaterial(mat1);
      // MaterialTable::Material mat2_data = material_table->GetMaterial(mat2);
      // Material gpumat1 = mat1_data;
      // Material gpumat2 = mat2_data;

      // material_buffer.ZeroMem();
      // material_buffer.BufferData(gpumat1);
      // material_buffer.BufferData(gpumat2);

      // material_uniforms->BindBase();
      // material_uniforms->LoadFromBuffer(material_buffer);

      shape.Draw(other::TRIANGLES);

      mat_shader->Bind();
      // default_shader->Bind();

      material_table->Unbind();
      frame->UnbindFrame();
      /// > GEOMETRY PASS

      /// > DRAW TO SCREEN
      other::Renderer::DrawFramebufferToWindow(frame);
      /// > DRAW TO SCREEN

#define UI_ENABLED 0
#if UI_ENABLED
      other::UI::BeginFrame();

      if (ImGui::Begin("GBuffer")) {
        RenderItem(gbuffer.textures[0], "Position", ImVec2((float)win_w / 2, (float)win_h / 2));
        RenderItem(gbuffer.textures[1], "Normals", ImVec2((float)win_w / 2, (float)win_h / 2));
        RenderItem(gbuffer.textures[2], "Albedo", ImVec2((float)win_w / 2, (float)win_h / 2));
      }
      ImGui::End();
      other::UI::EndFrame();
#endif
      other::Renderer::GetWindow()->SwapBuffers();
    }

    glDeleteProgram(shader1);
    glDeleteProgram(shader2);

    AppState::DetachApplication();
    PhysicsEngine::Shutdown();
    ScriptEngine::Shutdown();
    UI::Shutdown();
    Renderer::Shutdown();
    AppState::Shutdown();
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
    exit = 1;
  } catch (const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
    exit = 1;
  } catch (...) {
    std::cout << "unknown error" << "\n";
    exit = 1;
  }
  std::cout << "Exiting\n";
  return exit;
}

Shape::Shape(const Path& path, Ref<MaterialTable>& material_table) {
  OE_DEBUG("Attempting to load model : {}", path);
  Assimp::Importer importer;

  /// others

  uint32_t flags =
    aiProcess_CalcTangentSpace |          // Create binormals/tangents just in case
    aiProcess_Triangulate |               // Make sure we're triangles
    aiProcess_SortByPType |               // Split meshes by primitive type
    aiProcess_GenNormals |                // Make sure we have legit normals
    aiProcess_GenUVCoords |               // Convert UVs if required
                                          //		aiProcess_OptimizeGraph |
    aiProcess_RemoveRedundantMaterials |  // remove redundant materials
    aiProcess_FindDegenerates |           // remove degenerated polygons from the import
    aiProcess_FindInvalidData |           // detect invalid model data, such as invalid normal vectors
    aiProcess_TransformUVCoords |         // preprocess UV transformations (scaling, translation ...)
    aiProcess_FindInstances |             // search for instanced meshes and remove them by references to one master
    aiProcess_SplitByBoneCount |          // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
    aiProcess_OptimizeMeshes |            // Batch draws where possible
    aiProcess_JoinIdenticalVertices |
    aiProcess_LimitBoneWeights |       // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
    aiProcess_ValidateDataStructure |  // Validation
    aiProcess_GlobalScale;             // e.g. convert cm to m for fbx import (and other formats where cm is native)
  const aiScene* scene = importer.ReadFile(path.string(), flags);
  if (scene == nullptr) {
    OE_ERROR("Failed to load model : {}", path);
    throw std::runtime_error("Failed to load model");
  }

  if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
    OE_ERROR("Failed to load model : {}\n[ASSIMP ERROR : {}]", path, importer.GetErrorString());
    throw std::runtime_error("Failed to load model");
  }

  ProcessNode(scene->mRootNode, scene);
  GetMaterials(scene);
}

void Shape::Draw(DrawMode mode) {
  OE_ASSERT(mesh_vao != nullptr, "Mesh VAO is null");

  Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
  OE_ASSERT(material_table != nullptr, "Material table is null");

  mesh_vao->Bind();
  material_table->Bind();
  for (const SubMesh& sm : submeshes) {
    uint32_t base_vertex = sm.base_vertex;
    uint32_t idx_cnt = sm.idx_cnt;

    // other::UUID material_id = sm.material_id.Get() == 0 ? material_table->DefaultMaterial() : sm.material_id;
    // OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
    // Material gpumat = material_table->GetMaterial(material_id);

    glDrawElementsInstancedBaseVertexBaseInstance(mode, idx_cnt, GL_UNSIGNED_INT, (void*)0, 1, base_vertex, 0);
  }
  material_table->Unbind();
  mesh_vao->Unbind();
}

void Shape::ProcessNode(const aiNode* node, const aiScene* scene) {
  // process all the node's meshes (if any)
  for (uint32_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    if (mesh == nullptr) {
      OE_ERROR("Failed to get mesh : {}", node->mMeshes[i]);
      throw std::runtime_error("Failed to get mesh");
    }

    ProcessMesh(mesh, scene);
  }

  // then do the same for each of its children
  for (uint32_t i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }

  /// build mesh nodes
  // MeshNode& mesh_node = nodes.emplace_back();
  // TraverseNodes(scene->mRootNode, 0);

  for (const auto& submesh : submeshes) {
    BBox transformed_submesh_bounds = submesh.bounds;

    glm::vec3 min = glm::vec3(submesh.transform * glm::vec4(transformed_submesh_bounds.min, 1.0f));
    glm::vec3 max = glm::vec3(submesh.transform * glm::vec4(transformed_submesh_bounds.max, 1.0f));

    mesh_bounds.min.x = glm::min(mesh_bounds.min.x, min.x);
    mesh_bounds.min.y = glm::min(mesh_bounds.min.y, min.y);
    mesh_bounds.min.z = glm::min(mesh_bounds.min.z, min.z);

    mesh_bounds.max.x = glm::max(mesh_bounds.max.x, max.x);
    mesh_bounds.max.y = glm::max(mesh_bounds.max.y, max.y);
    mesh_bounds.max.z = glm::max(mesh_bounds.max.z, max.z);
  }
}

void Shape::ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
  OE_ASSERT(scene != nullptr, "Attempting to process a mesh without a scene");
  OE_ASSERT(mesh != nullptr, "Attempting to process a null mesh");

  SubMesh& submesh = submeshes.emplace_back();
  submesh.sub_mesh_id = Random::GenerateUUID();
  submesh.model_name = mesh->mName.C_Str();
  submesh.base_vertex = vertices.size();
  submesh.base_idx = indices.size();
  submesh.material_id = mesh->mMaterialIndex;
  submesh.idx_cnt = mesh->mNumFaces * 3;
  submesh.vert_cnt = mesh->mNumVertices;

  BBox& bounds = submesh.bounds;
  bounds = BBox::empty;
  for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
    Vertex& v = vertices.emplace_back();
    v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
    submesh.bounds.min.x = glm::min(v.position.x, submesh.bounds.min.x);
    submesh.bounds.min.y = glm::min(v.position.y, submesh.bounds.min.y);
    submesh.bounds.min.z = glm::min(v.position.z, submesh.bounds.min.z);

    submesh.bounds.max.x = glm::max(v.position.x, submesh.bounds.max.x);
    submesh.bounds.max.y = glm::max(v.position.y, submesh.bounds.max.y);
    submesh.bounds.max.z = glm::max(v.position.z, submesh.bounds.max.z);

    if (mesh->HasNormals()) {
      v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
    } else {
      v.normal = { 0.f, 0.f, 0.f };
    }

    if (mesh->HasTangentsAndBitangents()) {
      v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
      v.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
    } else {
      v.tangent = { 0.f, 0.f, 0.f };
      v.bitangent = { 0.f, 0.f, 0.f };
    }

    if (mesh->HasTextureCoords(0)) {
      v.uv_coord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
    } else {
      v.uv_coord = { 0.f, 0.f };
    }

    raw_vertices.push_back(v.position.x);
    raw_vertices.push_back(v.position.y);
    raw_vertices.push_back(v.position.z);
    raw_vertices.push_back(v.normal.x);
    raw_vertices.push_back(v.normal.y);
    raw_vertices.push_back(v.normal.z);
    raw_vertices.push_back(v.tangent.x);
    raw_vertices.push_back(v.tangent.y);
    raw_vertices.push_back(v.tangent.z);
    raw_vertices.push_back(v.bitangent.x);
    raw_vertices.push_back(v.bitangent.y);
    raw_vertices.push_back(v.bitangent.z);
    raw_vertices.push_back(v.uv_coord.x);
    raw_vertices.push_back(v.uv_coord.y);
  }

  for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    OE_ASSERT(face.mNumIndices == 3, "Other Engine does not support untriangulated meshes");
    Index& idx = indices.emplace_back();
    idx = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };

    raw_indices.push_back(face.mIndices[0]);
    raw_indices.push_back(face.mIndices[1]);
    raw_indices.push_back(face.mIndices[2]);
  }
}

void Shape::TraverseNodes(const aiNode* anode, int32_t node_idx, const glm::mat4& parent_transform, uint32_t level) {
  MeshNode& node = nodes[node_idx];
  node.name = anode->mName.C_Str();
  node.local_transform = glm::mat4(1.f);  // Utils::Mat4FromAIMatrix4x4(aNode->mTransformation);

  glm::mat4 transform = parent_transform * node.local_transform;
  for (uint32_t i = 0; i < anode->mNumMeshes; i++) {
    uint32_t idx = anode->mMeshes[i];
    SubMesh& submesh = submeshes[idx];
    submesh.model_name = anode->mName.C_Str();
    submesh.transform = transform;
    submesh.local_transform = node.local_transform;

    node.sub_meshes.push_back(idx);
  }

  uint32_t parent_node_idx = (uint32_t)nodes.size() - 1;
  node.children.resize(anode->mNumChildren);
  for (uint32_t i = 0; i < anode->mNumChildren; i++) {
    MeshNode& child = nodes.emplace_back();
    size_t child_idx = nodes.size() - 1;
    child.parent = parent_node_idx;
    nodes[node_idx].children[i] = child_idx;
    TraverseNodes(anode->mChildren[i], uint32_t(child_idx), transform, level + 1);
  }
}

void Shape::GetMaterials(const aiScene* scene) {
  OE_ASSERT(scene != nullptr, "Attempting to build a material table without a scene");

  Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
  OE_ASSERT(material_table != nullptr, "Material table is null");

  for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
    aiMaterial* material = scene->mMaterials[i];
    if (material == nullptr) {
      OE_ERROR("Failed to get material : {}", i);
      throw std::runtime_error("Failed to get material");
    }

    aiString mat_name;
    material->Get(AI_MATKEY_NAME, mat_name);

    OE_DEBUG("  {0} (Index = {1})", mat_name.data, i);
    aiString tex_path;
    uint32_t textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
    OE_DEBUG("    TextureCount = {0}", textureCount);

    Opt<glm::vec4> albedo_col = std::nullopt;
    Opt<float> emission = std::nullopt;

    aiColor3D col;
    aiColor3D emissive;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == AI_SUCCESS) {
      albedo_col = { col.r, col.g, col.b, 1.f };
    }

    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
      emission = emissive.r;
    }

    float roughness;
    float metalness;
    if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS) {
      roughness = 0.5f;  // Default value
    }

    if (material->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS) {
      metalness = 0.0f;
    }

    OE_DEBUG("    COLOR = {0}, {1}, {2}", col.r, col.g, col.b);
    OE_DEBUG("    ROUGHNESS = {0}", roughness);
    OE_DEBUG("    METALNESS = {0}", metalness);
    bool has_albedo_map = material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS;
    bool fallback = !has_albedo_map;

    Opt<MaterialTable::Texture> albedo_tex = std::nullopt;
    Opt<MaterialTable::Texture> normal_tex = std::nullopt;
    Opt<MaterialTable::Texture> roughness_tex = std::nullopt;

    if (has_albedo_map) {
      if (const aiTexture* tex = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(tex->pcData);
        albedo_tex = MaterialTable::CreateTexture(4 * tex->mWidth * tex->mHeight, data, { tex->mWidth, tex->mHeight });
      }
      /// if texture not embedded, attempt loading from file if it exists
      else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
        Ref<FileHandle> file = Filesystem::GetFile(full_path);
        if (file != nullptr) {
          std::vector<uint8_t> data = file->ReadBytes();
          if (data.empty()) {
            OE_ERROR("Failed to get texture  from file : {}", full_path);
            fallback = !albedo_col.has_value();
          } else {
            albedo_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
          }
        } else {
          OE_ERROR("Failed to get file handle : {}", full_path);
          fallback = !albedo_col.has_value();
        }
      }
      /// file does not exist, fallback to color
      else {
        OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
        fallback = !albedo_col.has_value();
      }
    }

    if (fallback) {
      albedo_tex = MaterialTable::CreateTexture(glm::vec4{ 1.f }, { 1920.f, 1080.f });
    } else if (albedo_col.has_value()) {
      albedo_tex = MaterialTable::CreateTexture(albedo_col.value(), { 1920.f, 1080.f });
    }

    // Normal maps
    bool has_normal_map = material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS;
    fallback = !has_normal_map;
    if (has_normal_map) {
      if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
        normal_tex = MaterialTable::CreateTexture(4 * texture->mWidth * texture->mHeight, data, { texture->mWidth, texture->mHeight });
      } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
        Ref<FileHandle> file = Filesystem::GetFile(full_path);
        if (file != nullptr) {
          std::vector<uint8_t> data = file->ReadBytes();
          if (data.empty()) {
            OE_ERROR("Failed to get texture  from file : {}", full_path);
            fallback = true;
          } else {
            normal_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
          }
        } else {
          OE_ERROR("Failed to get file handle : {}", full_path);
          fallback = true;
        }
      } else {
        OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
        fallback = true;
      }
    }

    if (fallback) {
      normal_tex = MaterialTable::CreateTexture(glm::vec4{ 0.5f, 0.5f, 1.f, 1.f }, { 1920.f, 1080.f });
    }

    // Roughness map
    bool has_roughness_map = material->GetTexture(aiTextureType_SHININESS, 0, &tex_path) == AI_SUCCESS;
    fallback = !has_roughness_map;
    if (has_roughness_map) {
      if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
        roughness_tex = MaterialTable::CreateTexture(4 * texture->mWidth * texture->mHeight, data, { texture->mWidth, texture->mHeight });
      } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
        Ref<FileHandle> file = Filesystem::GetFile(full_path);
        if (file != nullptr) {
          std::vector<uint8_t> data = file->ReadBytes();
          if (data.empty()) {
            OE_ERROR("Failed to get texture  from file : {}", full_path);
            fallback = true;
          } else {
            roughness_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
          }
        } else {
          OE_ERROR("Failed to get file handle : {}", full_path);
          fallback = true;
        }
      } else {
        OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
        fallback = true;
      }
    }

    if (fallback) {
      roughness_tex = MaterialTable::CreateTexture(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.f }, { 1920.f, 1080.f });
    }

    OE_ASSERT(albedo_tex.has_value(), "Albedo texture is null");
    OE_ASSERT(normal_tex.has_value(), "Normal texture is null");
    OE_ASSERT(roughness_tex.has_value(), "Roughness texture is null");

    other::UUID mat_id = material_table->RegisterMaterial(albedo_tex.value(), normal_tex.value(), roughness_tex.value());
    OE_DEBUG("    Material ID = {0}", mat_id);
    material_ids.push_back(mat_id);
  }
}

Quad::Quad() {
  vertices = {
    /* (-,+,0) */ -1.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    /* (-,-,0) */ -1.0f,
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    /* (+,+,0) */ 1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    /* (+,-,0) */ 1.0f,
    -1.0f,
    0.0f,
    1.0f,
    0.0f,
  };
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

Quad::~Quad() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}

void Quad::Draw() {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

// glm::vec3 position
// glm::vec3 normal
// glm::vec3 tangent
// glm::vec3 bitangent
// glm::vec2 uv_coord

Cube::Cube() {
  // clang-format off
  vertices = {
    /* (-,-,+) */
    -1.0f / 2.0f, -1.0f / 2.0f, 1.0f / 2.0f,
    -1.f, -1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 1.f,
    /* (+,-,+) */ 
    1.0f / 2.0f, -1.0f / 2.0f, 1.0f / 2.0f,
    1.f, -1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 1.f,
    /* (+,+,+) */ 
    1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f,
    1.f, 1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 0.f,
    /* (-,+,+) */ 
    -1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f,
    -1.f, 1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 0.f,
    /* (-,-,-) */ 
    -1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f,
    -1.f, -1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 1.f,
    /* (+,-,-) */ 
    1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f,
    1.f, -1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 1.f,
    /* (+,+,-) */ 
    1.0f / 2.0f, 1.0f / 2.0f, -1.0f / 2.0f,
    1.f, 1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 0.f,
    /* (-,+,-) */ 
    -1.0f / 2.0f, 1.0f / 2.0f, -1.0f / 2.0f,
    -1.f, 1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 0.f,
  };
  // clang-format on
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  uint32_t stride = 14;

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(9 * sizeof(float)));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(12 * sizeof(float)));

  glBindVertexArray(0);
}

Cube::~Cube() {
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

void Cube::Draw(other::DrawMode mode, uint32_t instances) {
  glBindVertexArray(vao);
  glDrawElementsInstancedBaseVertexBaseInstance(mode, 36, GL_UNSIGNED_INT, (void*)0, instances, 0, 0);
  glBindVertexArray(0);
}

void UpdateCamera(other::Ref<CameraBase>& camera) {
  if (other::Keyboard::Down(other::Keyboard::Key::OE_W)) {
    camera->MoveForward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_S)) {
    camera->MoveBackward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_A)) {
    camera->MoveLeft();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_D)) {
    camera->MoveRight();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_SPACE)) {
    camera->MoveUp();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_LSHIFT)) {
    camera->MoveDown();
  }

  glm::vec2 win_size = { win_w, win_h };
  glm::ivec2 mouse_pos = other::Mouse::GetPos();

  SDL_WarpMouseInWindow(SDL_GetMouseFocus(), win_size.x / 2, win_size.y / 2);

  camera->SetLastMouse(camera->Mouse());
  camera->SetMousePos(mouse_pos);
  camera->SetDeltaMouse({ camera->Mouse().x - camera->LastMouse().x,
                          camera->LastMouse().y - camera->Mouse().y });

  glm::ivec2 rel_pos = other::Mouse::GetRelPos();

  camera->SetYaw(camera->Yaw() + (rel_pos.x * camera->Sensitivity()));
  camera->SetPitch(camera->Pitch() - (rel_pos.y * camera->Sensitivity()));

  if (camera->ConstrainPitch()) {
    if (camera->Pitch() > 89.0f) {
      camera->SetPitch(89.0f);
    }

    if (camera->Pitch() < -89.0f) {
      camera->SetPitch(-89.0f);
    }
  }

  camera->UpdateCoordinateFrame();
  camera->CalculateMatrix();
}
