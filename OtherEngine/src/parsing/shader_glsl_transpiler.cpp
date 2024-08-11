/**
 * \file parsing/shader_glsl_transpiler.cpp
 **/
#include "parsing/shader_glsl_transpiler.hpp"

#include "core/errors.hpp"
#include "parsing/parsing_defines.hpp"
#include "parsing/shader_ast_node.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {
      
  ShaderIr ShaderGlslTranspiler::Transpile() {
    ProcessNodes();

    std::vector<Ref<AstNode>>* correct_nodes = nullptr;
    std::string* output_src = nullptr;

    bool other_shader = false;
    switch (ast.type) {
      case VERTEX_SHADER:
        correct_nodes = &ast.vertex_nodes;
        output_src = &result.vert_source;
        break;
      case FRAGMENT_SHADER:
        correct_nodes = &ast.fragment_nodes;
        output_src = &result.frag_source;
        break;
      case GEOMETRY_SHADER:
        if (result.geom_source.has_value()) {
          correct_nodes = &ast.geometry_nodes;
          output_src = &result.geom_source.value();
        }
        break;
      case OTHER_SHADER:
        other_shader = true;
        break;
      default:
        break;
    }

    if (!other_shader) {
      if (correct_nodes == nullptr || output_src == nullptr) {
        throw ShaderException("GLSL Transpiler state corrupted, transpiling invalid shader!" , INVALID_SHADER_CTX , 0 , 0);
      }

      *output_src = TranspileTo(*correct_nodes); 
    } else {
      context = VERTEX_SHADER;
      result.vert_source = TranspileTo(ast.vertex_nodes); 
      
      context = FRAGMENT_SHADER;
      result.frag_source = TranspileTo(ast.fragment_nodes); 
      
      if (ast.geometry_nodes.size() > 0) {
        context = GEOMETRY_SHADER;
        result.geom_source = TranspileTo(ast.geometry_nodes); 
      }
    }

    if (!mesh_layout.has_value()) {
      throw Error(SHADER_TRANSPILATION , "Failed to set mesh layout parsing shader!");
    }
    result.layout = mesh_layout.value();

    return result;
  }

  void ShaderGlslTranspiler::Visit(LiteralExpr& expr) {
    token_stack.push(expr.value);
  }

  void ShaderGlslTranspiler::Visit(UnaryExpr& expr) {
    expr.right->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(BinaryExpr& expr) {
    expr.left->Accept(*this);
    expr.right->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(CallExpr& expr) {
    expr.callee->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(GroupingExpr& expr) {
    expr.expr->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(VarExpr& expr) {
  }

  void ShaderGlslTranspiler::Visit(AssignExpr& expr) {
    expr.value->Accept(*this);  
  }

  void ShaderGlslTranspiler::Visit(ArrayExpr& expr) {
    for (auto& val : expr.elements) {
      val->Accept(*this);
    }
  }

  void ShaderGlslTranspiler::Visit(ArrayAccessExpr& expr) {
    expr.array->Accept(*this);
    expr.index->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(ObjAccessExpr& expr) {
    expr.obj->Accept(*this);
    if (expr.assignment != nullptr) {
      expr.assignment->Accept(*this);
    }

    if (expr.index != nullptr) {
      expr.index->Accept(*this);
    }
  }
  
  void ShaderGlslTranspiler::Visit(ExprStmt& stmt) {
    stmt.expr->Accept(*this);
  }

  void ShaderGlslTranspiler::Visit(VarDecl& stmt) {
    if (!flags.is_uniform.empty()) {
      Uniform uni{
        .name = stmt.name.value , 
        .type = ValueTypeFromString(stmt.type.value) ,
      }; 
    
      if(!flags.push_uniforms.empty()) {
        uniform_stack.push(uni);
      } else {
        result.uniforms[FNV(uni.name)] = uni;
      }
      return;
    }
      

    if (stmt.initializer != nullptr) {
      stmt.initializer->Accept(*this); 
    } 
  }

  void ShaderGlslTranspiler::Visit(ArrayDecl& stmt) {
    if (stmt.initializer != nullptr) {
      stmt.initializer->Accept(*this); 
    } 
  }

  void ShaderGlslTranspiler::Visit(BlockStmt& stmt) {
    for (auto& s : stmt.statements) {
      s->Accept(*this);
    }
  }

  void ShaderGlslTranspiler::Visit(IfStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(WhileStmt& stmt) {
  }

  void ShaderGlslTranspiler::Visit(ReturnStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(FunctionStmt& stmt) {
    if (stmt.body != nullptr) {
      stmt.body->Accept(*this);
    }
  }

  void ShaderGlslTranspiler::Visit(StructStmt& stmt) {}

  void ShaderGlslTranspiler::Visit(VersionExpr& expr) {
  }
      
  void ShaderGlslTranspiler::Visit(ShaderAttribute& expr) {
    if (expr.key.value == "mesh") {
      SetMeshLayout(expr.value.value);
    }
  }

  void ShaderGlslTranspiler::Visit(LayoutDescriptor& expr) {
    expr.expr->Accept(*this);

    if (expr.type.type == LOCATION_KW) {
      if (token_stack.size() != 1) {
        throw Error(SHADER_TRANSPILATION , "Corrupt token stack transpiling layout location descriptor! size = {}" , token_stack.size());
      }

      Token literal = token_stack.top();
      token_stack.pop();

      if (mesh_layout.has_value() && mesh_layout->override) {
         return;
      }

      if (!mesh_layout.has_value()) {
        mesh_layout = MeshLayout{};
      }

      /// curr_idx and what we're reading will always be the same
      vertex_attr_stack.push({
        .idx = std::stoul(literal.value) , 
      });

      mesh_layout->curr_idx++;
    }

    if (expr.type.type == BINDING_KW) {
      if (token_stack.size() != 1) {
        throw Error(SHADER_TRANSPILATION , "Corrupt token stack transpiling layout binding descriptor! size = {}" , token_stack.size());
      }

      Token literal = token_stack.top();
      token_stack.pop();

      shader_storage_stack.push({
        .binding_point = std::stoul(literal.value) , 
      });
    }
  }

  void ShaderGlslTranspiler::Visit(LayoutDecl& stmt) {
    for (auto& d : stmt.descriptors) {
      d->Accept(*this);
    }

    stmt.data->Accept(*this);

    if (shader_storage_stack.size() == 1) {
      if (stmt.layout_rules.value == "std140") {
        shader_storage_stack.top().type = STD140;
      } else if (stmt.layout_rules.value == "std430") {
        shader_storage_stack.top().type = STD430;
      }

      auto storage = shader_storage_stack.top();
      shader_storage_stack.pop();

      result.storages[FNV(storage.name)] = storage;
    }

    if (vertex_attr_stack.size() == 1 && token_stack.size() == 2) {
      if (context != VERTEX_SHADER) {
        throw Error(INVALID_SHADER_CTX , "Can not write vertex attributes from anywhere but the vertex shader");
      }

      auto attr = vertex_attr_stack.top();
      vertex_attr_stack.pop();

      Token name = token_stack.top();
      token_stack.pop();
      Token type = token_stack.top();
      token_stack.pop();

      attr.attr_name = name.value;
      attr.size = SizeFromString(type.value);

      if (!mesh_layout.has_value()) {
        throw Error(SHADER_TRANSPILATION , "Corrupted transpiler state");
      }

      mesh_layout->stride += attr.size;
      mesh_layout->attrs.push_back(attr);
    } else if (false) {
    }

    while (!token_stack.empty()) {
      token_stack.pop();
    }
  }
      
  void ShaderGlslTranspiler::Visit(LayoutVarDecl& stmt) {
    token_stack.push(stmt.type.value_or(Token({} , INVALID_TOKEN , "")));
    token_stack.push(stmt.name.value_or(Token({} , INVALID_TOKEN , "")));
  }

  void ShaderGlslTranspiler::Visit(ShaderStorageStmt& stmt) {
    if (shader_storage_stack.size() != 1) {
      throw Error(SHADER_TRANSPILATION , "Shader storage stack is corrupt parsing shader uniforms!");
    }  

    flags.push_uniforms.push(true);
    stmt.body->Accept(*this);
    flags.push_uniforms.pop();

    ShaderStorage& storage = shader_storage_stack.top();
    while (!uniform_stack.empty()) {
      auto uni = uniform_stack.top();
      uniform_stack.pop();

      storage.uniforms[FNV(uni.name)] = uni;
    }
  }

  void ShaderGlslTranspiler::Visit(InOutBlockStmt& stmt) {
    stmt.body->Accept(*this); 
  }

  void ShaderGlslTranspiler::Visit(UniformDecl& stmt) {
    flags.is_uniform.push(true);
    stmt.var_decl->Accept(*this);
    flags.is_uniform.pop();
  }

  void ShaderGlslTranspiler::Visit(ShaderDecl& stmt) {
    context = stmt.type;

    for (auto& a : stmt.attributes) {
      a->Accept(*this);
    }

    for (auto& s : stmt.statements) {
      s->Accept(*this);
    } 
  }

  std::string ShaderGlslTranspiler::TranspileTo(std::vector<Ref<AstNode>>& nodes) {
    std::stringstream stream;
    stream << "#version 460 core\n\n";

    /// define this in both vertex and fragment 
    if (context == VERTEX_SHADER || context == FRAGMENT_SHADER) {
      stream << "struct Material {\n";
      stream << "  vec4 ambient;\n";
      stream << "  vec4 diffuse;\n";
      stream << "  vec4 specular;\n";
      stream << "  float shininess;\n";
      stream << "};\n\n";
    }

    /// lights only necessary in fragment
    if (context == FRAGMENT_SHADER) {
      stream << "struct PointLight {\n";
      stream << "  vec4 position;\n";
      stream << "  vec4 ambient;\n";
      stream << "  vec4 diffuse;\n";
      stream << "  vec4 specular;\n";
      stream << "  float constant;\n";
      stream << "  float linear;\n";
      stream << "  float quadratic;\n";
      stream << "};\n\n";
      
      stream << "struct DirectionLight {\n";
      stream << "  vec4 direction;\n";
      stream << "  vec4 ambient;\n";
      stream << "  vec4 diffuse;\n";
      stream << "  vec4 specular;\n";
      stream << "};\n\n";
    }

    std::stringstream ts;
    if (mesh_layout.has_value() && mesh_layout->override && context == VERTEX_SHADER) {
      for (const auto& a : mesh_layout.value().attrs) {
        stream << "layout (location = " << a.idx << ") in ";
        switch (a.size) {
          case 1:
            stream << "int ";
            break;
          case 2:
            stream << "vec2 ";
            break;
          case 3:
            stream << "vec3 ";
            break;
          case 4:
            stream << "vec4 ";
            break;
          default:
            break;
        }
        stream << a.attr_name << ";\n";
      }
      stream << "\n";
    }

    if (context == VERTEX_SHADER) {
      stream << "layout (std140 , binding = 0) uniform Camera {\n";
      stream << "  mat4 projection;\n";
      stream << "  mat4 view;\n";
      stream << "  vec4 viewpoint;\n";
      stream << "};\n\n";
      stream << "layout (std430 , binding = 1) readonly buffer ModelData {\n";
      stream << "  mat4 models[];\n";
      stream << "};\n\n";
      stream << "layout (std430 , binding = 2) readonly buffer MaterialData {\n";
      stream << "  Material materials[];\n";
      stream << "};\n\n";
      stream << "out Material material;\n\n";
    } else if (context == FRAGMENT_SHADER) {
      stream << "in Material material;\n\n";
      stream << "#define MAX_POINT_LIGHTS 100\n";
      stream << "#define MAX_DIR_LIGHTS 100\n";
      stream << "layout (std430 , binding = 3) readonly buffer Lights {\n";
      /// num direction lights is num_lights.x
      /// num point lights is num_lights.y
      /// num_lights.z and .w are padding
      stream << "  vec4 num_lights;\n";
      stream << "  DirectionLight direction_lights[MAX_DIR_LIGHTS];\n";
      stream << "  PointLight point_lights[MAX_POINT_LIGHTS];\n";
      stream << "};\n\n";
    }

    for (auto& n : nodes) {
      n->Stream(stream , *this);
    }

    return stream.str();
  }
    
  void ShaderGlslTranspiler::ProcessNodes() {
    context = VERTEX_SHADER;
    for (auto& n : ast.vertex_nodes) {
      n->Accept(*this);   
    }
    
    context = FRAGMENT_SHADER;
    for (auto& n : ast.fragment_nodes) {
      n->Accept(*this);   
    }

    if (ast.geometry_nodes.size() > 0) {
      context = GEOMETRY_SHADER;
      for (auto& n : ast.fragment_nodes) {
        n->Accept(*this);   
      }
    }
  }
  
  constexpr static uint64_t kDefaultMeshHash = FNV("default");
  const static MeshLayout kDefaultMesh = {
    .layout_name = "default" ,
    .stride = 9 ,
    .attrs = {
      { .attr_name = "voe_position"     , .idx = 0 , .size = 3 },
      { .attr_name = "voe_normal"    , .idx = 1 , .size = 3 },
      { .attr_name = "voe_tangent"     , .idx = 2 , .size = 3 },
      { .attr_name = "voe_bitangent"   , .idx = 3 , .size = 3 },
      { .attr_name = "voe_uvs"     , .idx = 4 , .size = 2 },
    } ,
  };

  constexpr static uint64_t kTexturedQuadMeshHash = FNV("textured_quad");
  const static MeshLayout kTexturedQuadMesh = {
    .layout_name = "textured_quad" ,
    .stride = 5 , 
    .attrs = {
      { .attr_name = "voe_position" , .idx = 0 , .size = 2 } ,
      { .attr_name = "voe_uvs" , .idx = 1 , .size = 2 } ,
    } ,
  };
      
  void ShaderGlslTranspiler::SetMeshLayout(const std::string& value) {
    if (context != VERTEX_SHADER) {
      throw Error(INVALID_SHADER_CTX , "Can not change mesh layout from anything but a vertex shader!");
    }
    
    /// only allow one override
    if (mesh_layout.has_value() && mesh_layout->override) {
      return; 
    }

    /// if we haven't come down this path before but there is a mesh layout, issue warning 
    if (mesh_layout.has_value() && !mesh_layout->override) {
      OE_WARN("Overwriting provided input layout because mesh attribute {} was present in shader attributes" , value);
    }

    switch (FNV(value)) {
      case kDefaultMeshHash:
        mesh_layout = kDefaultMesh;
        break;
      case kTexturedQuadMeshHash:
        mesh_layout = kTexturedQuadMesh;
        break;
      default:
        if (!mesh_layout.has_value()) {
          OE_ERROR("Invalid mesh in vertex shader attibutes!");
          return;
        } 
        break;
    }

    OE_INFO("Set mesh layout to {}" , value);

    /// mark we have already set one mesh attribute
    mesh_layout->override = true;
  }
      
  uint32_t ShaderGlslTranspiler::SizeFromString(const std::string& str) {
    if (str == "int") {
      return 1;
    } else if (str == "vec2") {
      return 2;
    } else if (str == "vec3") {
      return 3;
    } else if (str == "vec4") {
      return 4;
    } else if (str == "mat2") {
      return 2 * 2;
    } else if (str == "mat3") {
      return 3 * 3;
    } else if (str == "mat4") {
      return 4 * 4;
    }
    return 0;
  }
      
  ValueType ShaderGlslTranspiler::ValueTypeFromString(const std::string& str) {
    if (str == "int") {
      return INT32;
    } else if (str == "vec2") {
      return VEC2;
    } else if (str == "vec3") {
      return VEC3;
    } else if (str == "vec4") {
      return VEC4;
    } else if (str == "mat2") {
      return MAT2;
    } else if (str == "mat3") {
      return MAT3;
    } else if (str == "mat4") {
      return MAT4;
    }
    return EMPTY;
  }

} // namespace other
