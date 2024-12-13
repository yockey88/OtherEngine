/**
 * \file serialization_tests.cpp
 **/
#include <string>

#include <gtest.h>

#include "core/defines.hpp"
#include "core/writer_reader.hpp"

#include "rendering/framebuffer.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/rendering_defines.hpp"

#include "oetest.hpp"

using other::OtherTest;
using other::Reader;
using other::Writer;

using other::FramebufferSpec;
using other::PipelineSpec;
using other::RenderPassSpec;
using other::Uniform;

using other::ValueType;

class SerializationTests : public OtherTest {
 public:
  virtual ~SerializationTests() override = default;
};

TEST_F(SerializationTests, Reader) {
  {
    std::string data = " {  1 ,  2,      3 ,}";
    std::stringstream ss(data);

    glm::vec3 v = Reader<glm::vec3>{}(ss);
    EXPECT_EQ(v.x, 1.f);
    EXPECT_EQ(v.y, 2.f);
    EXPECT_EQ(v.z, 3.f);
  }

  {
    std::string data = "{1 , 2}";
    std::stringstream ss(data);

    glm::vec2 v = Reader<glm::vec2>{}(ss);
    EXPECT_EQ(v.x, 1.f);
    EXPECT_EQ(v.y, 2.f);
  }

  {
    std::string data = "\n\n1           ";
    std::stringstream ss(data);

    int i = Reader<int>{}(ss);
    EXPECT_EQ(i, 1);
  }

  {
    std::vector<int> expected{ 1, 2, 3, 4, 5 };
    std::string data = "1\n2\n3\n4\n5\n";
    std::stringstream ss(data);

    std::vector<int> v = Reader<std::vector<int>>{}(ss);
    for (size_t i = 0; i < v.size(); i++) {
      EXPECT_EQ(v[i], expected[i])
        << "Mismatch at index " << i << " expected: " << expected[i] << " got: " << v[i];
    }
  }

  {
    std::vector<std::string> expected{ "hello", "world" };
    std::string data = "hello\nworld\n";
    std::stringstream ss(data);

    std::vector<std::string> v = Reader<std::vector<std::string>>{}(ss);
    for (size_t i = 0; i < v.size(); i++) {
      EXPECT_EQ(v[i], expected[i])
        << "Mismatch at index " << i << " expected: " << expected[i] << " got: " << v[i];
    }
  }

  {
    glm::vec3 expected{ 1.f, 2.f, 3.f };
    std::string data = R"(
      {
        1,
        2,
        3
      }
    )";
    std::stringstream ss(data);
    glm::vec3 v = Reader<glm::vec3>{}(ss);

    EXPECT_EQ(v, expected);
  }

  {
    glm::vec2 expected{ 1.f, 2.f };
    std::string data = R"(
      {
        1,
        2
      }
    )";
    std::stringstream ss(data);
    glm::vec2 v = Reader<glm::vec2>{}(ss);

    EXPECT_EQ(v, expected);
  }

  {
    std::vector<std::string> expected{ "hello", "world" };
    std::string data = R"(
      hello
      world
    )";
    std::stringstream ss(data);

    std::vector<std::string> v = Reader<std::vector<std::string>>{}(ss);
    for (size_t i = 0; i < v.size(); i++) {
      EXPECT_EQ(v[i], expected[i])
        << "Mismatch at index " << i << " expected: " << expected[i] << " got: " << v[i];
    }
  }

  {
    std::string data = "vec4";
    std::stringstream ss(data);

    ValueType t = Reader<ValueType>{}(ss);
    EXPECT_EQ(t, ValueType::VEC4);
  }

  {
    std::vector<std::string> expected{ "hello", "there", "world" };
    std::string data = R"(
      {
        hello            ,
        there
        ,world
      }
    )";
    std::stringstream ss(data);

    std::vector<std::string> v = Reader<std::vector<std::string>>{}(ss);
    for (size_t i = 0; i < v.size(); i++) {
      EXPECT_EQ(v[i], expected[i])
        << "Mismatch at index " << i << " expected: " << expected[i] << " got: " << v[i];
    }
  }

  {
    std::string expected_str = "string";
    std::string expected_key = "key";
    std::string expected_value = "value";
    std::string data = R"(
      string
      key = value
    )";
    std::stringstream ss(data);

    std::string str = Reader<std::string>{}(ss);
    EXPECT_EQ(str, expected_str);

    std::string key = Reader<std::string>{}(ss);
    EXPECT_EQ(key, expected_key);

    char c = Reader<char>{}(ss);
    EXPECT_EQ(c, '=');

    std::string value = Reader<std::string>{}(ss);
    EXPECT_EQ(value, expected_value);
  }

  {
    std::string expected_str = "string";
    std::string expected_key = "key";
    std::string expected_value = "value";
    std::string data = R"(
      string
      key = value
    )";
    std::stringstream ss(data);

    std::string str = Reader<std::string>{}(ss);
    EXPECT_EQ(str, expected_str);

    auto [key, value] = other::ReadKeyValue(ss, Reader<std::string>{});
    EXPECT_EQ(key, expected_key);
    EXPECT_EQ(value, expected_value);
  }
}

TEST_F(SerializationTests, Writer) {
  {
    std::stringstream ss;
    Writer<glm::vec3>{}(ss, glm::vec3(1.f, 2.f, 3.f));
    ASSERT_EQ(ss.str(), "{1,2,3}");
  }

  {
    std::stringstream ss;
    Writer<glm::vec2>{}(ss, glm::vec2(1.11f, 2.22f));
    ASSERT_EQ(ss.str(), "{1.11,2.22}");
  }

  {
    std::stringstream ss;
    Writer<glm::vec2>{}(ss, glm::vec2(1.f, 2.f));
    ASSERT_EQ(ss.str(), "{1,2}");
  }

  {
    std::stringstream ss;
    Writer<int>{}(ss, 1);
    ASSERT_EQ(ss.str(), "1");
  }

  {
    std::stringstream ss;
    Writer<std::string>{}(ss, "hello");
    ASSERT_EQ(ss.str(), "hello");
  }

  {
    std::vector<int> data{ 1, 2, 3, 4, 5 };
    std::stringstream ss;
    Writer<std::vector<int>>{}(ss, data);

    std::string expected = R"({
      1,
      2,
      3,
      4,
      5
  }
)";
    ASSERT_EQ(ss.str(), expected);
  }
}

TEST_F(SerializationTests, framebuffer_spec) {
  {
    std::string data = R"(
      Framebuffer
      depth-function = LESS
      clear-color = {1,1,1,1}
      size = {1920,1080}
      depth-buffer = true
      color-buffer = true
      stencil-buffer = false
    )";
    std::stringstream stream(data);

    auto name = Reader<std::string>{}(stream);
    EXPECT_EQ(name, "Framebuffer");

    auto [dfk, depth_func] = ReadKeyValue(stream, Reader<other::DepthFunction>{});
    EXPECT_EQ(dfk, "depth-function");
    EXPECT_EQ(depth_func, other::DepthFunction::LESS);

    auto [cck, clear_color] = ReadKeyValue(stream, Reader<glm::vec4>{});
    EXPECT_EQ(cck, "clear-color");
    EXPECT_EQ(clear_color, glm::vec4(1.f, 1.f, 1.f, 1.f));

    auto [szk, size] = ReadKeyValue(stream, Reader<glm::ivec2>{});
    EXPECT_EQ(szk, "size");
    EXPECT_EQ(size, glm::ivec2(1920, 1080));

    auto [dbk, depth] = ReadKeyValue(stream, Reader<bool>{});
    EXPECT_EQ(dbk, "depth-buffer");
    EXPECT_EQ(depth, true);

    auto [cbk, color] = ReadKeyValue(stream, Reader<bool>{});
    EXPECT_EQ(cbk, "color-buffer");
    EXPECT_EQ(color, true);

    auto [sbk, stencil] = ReadKeyValue(stream, Reader<bool>{});
    EXPECT_EQ(sbk, "stencil-buffer");
    EXPECT_EQ(stencil, false);
  }
  {
    std::stringstream ss;
    other::FramebufferSpec spec{
      .depth_func = other::DepthFunction::LESS,
      .clear_color = glm::vec4(1.f),
      .size = glm::ivec2(1920, 1080),
      .depth = true,
      .color = true,
      .stencil = false,
    };

    Writer<FramebufferSpec>{}(ss, spec);

    std::string expected_str = R"({
    Framebuffer
    depth-function = LESS
    clear-color = {1,1,1,1}
    size = {1920,1080}
    depth-buffer = true
    color-buffer = true
    stencil-buffer = false
  }
)";

    EXPECT_EQ(ss.str(), expected_str);
  }

  {
    std::string data = R"(
        {
          Framebuffer
          depth-function = LESS
          clear-color = {
            1, 1, 1, 1
          }
          size = {1920,1080}
          depth-buffer = true
          color-buffer = true
          stencil-buffer = false
        })";
    std::stringstream ss(data);

    FramebufferSpec spec;
    ASSERT_NO_FATAL_FAILURE(spec = Reader<FramebufferSpec>{}(ss));
    EXPECT_EQ(spec.depth_func, other::DepthFunction::LESS);
    EXPECT_EQ(spec.clear_color, glm::vec4(1.f, 1.f, 1.f, 1.f));
    EXPECT_EQ(spec.size, glm::ivec2(1920, 1080));
    EXPECT_EQ(spec.depth, true);
    EXPECT_EQ(spec.color, true);
    EXPECT_EQ(spec.stencil, false);
  }
}

TEST_F(SerializationTests, uniform_spec) {
  {
    std::stringstream ss;
    ValueType type = ValueType::VEC4;
    Writer<ValueType>{}(ss, type);
    EXPECT_EQ(ss.str(), "vec4");
  }

  {
    std::stringstream ss("vec4");
    ValueType t;
    ASSERT_NO_FATAL_FAILURE(t = Reader<ValueType>{}(ss));
    EXPECT_EQ(t, ValueType::VEC4);
  }

  {
    std::stringstream ss;
    other::Uniform uniform{
      .name = "color",
      .type = ValueType::VEC4,
      .arr_length = 1,
      .size = 16,
    };

    Writer<Uniform>{}(ss, uniform);

    std::string expected_str = "vec4:color";
    EXPECT_EQ(ss.str(), expected_str);

    std::string data = "vec4:color";
    ss = std::stringstream(data);

    Uniform u;
    ASSERT_NO_FATAL_FAILURE(u = Reader<Uniform>{}(ss));
    EXPECT_EQ(u.name, "color");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 1);
    EXPECT_EQ(u.size, 16);
  }

  {
    std::string data = R"(
      {
        vec4:color
      }
    )";
    std::stringstream ss(data);

    Uniform u;

    char c = Reader<char>{}(ss);
    ASSERT_EQ(c, '{');

    u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "color");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 1);
    EXPECT_EQ(u.size, 16);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, '}');
  }

  {
    std::string data = R"(
      {
        vec4:color,
        vec3:position
      }
    )";
    std::stringstream ss(data);

    Uniform u;

    char c = Reader<char>{}(ss);
    ASSERT_EQ(c, '{');

    u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "color");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 1);
    EXPECT_EQ(u.size, 16);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, ',');

    u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "position");
    EXPECT_EQ(u.type, ValueType::VEC3);
    EXPECT_EQ(u.arr_length, 1);
    EXPECT_EQ(u.size, 12);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, '}');
  }

  {
    std::string data = "vec4:models:100";
    std::stringstream ss(data);

    Uniform u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "models");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 100);
    EXPECT_EQ(u.size, 16);
  }

  {
    std::string data = R"(
      {
        vec4:models:100,
        vec3:position,
        vec4:colors:100
      }
    )";
    std::stringstream ss(data);

    char c = Reader<char>{}(ss);
    ASSERT_EQ(c, '{');

    Uniform u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "models");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 100);
    EXPECT_EQ(u.size, 16);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, ',');

    u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "position");
    EXPECT_EQ(u.type, ValueType::VEC3);
    EXPECT_EQ(u.arr_length, 1);
    EXPECT_EQ(u.size, 12);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, ',');

    u = Reader<Uniform>{}(ss);
    EXPECT_EQ(u.name, "colors");
    EXPECT_EQ(u.type, ValueType::VEC4);
    EXPECT_EQ(u.arr_length, 100);
    EXPECT_EQ(u.size, 16);

    c = Reader<char>{}(ss);
    ASSERT_EQ(c, '}');
  }

  {
    std::vector<Uniform> expected{
      { "color", ValueType::VEC4, 1, 16 },
    };

    std::string data = "{vec4:color}";
    std::stringstream ss(data);

    std::vector<Uniform> v = Reader<std::vector<Uniform>>{}(ss);

    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0].name, expected[0].name);
    EXPECT_EQ(v[0].type, expected[0].type);
    EXPECT_EQ(v[0].arr_length, expected[0].arr_length);
    EXPECT_EQ(v[0].size, expected[0].size);
  }

  {
    std::vector<Uniform> expected{
      { "color", ValueType::VEC4, 1, 16 },
      { "position", ValueType::VEC3, 1, 12 },
    };

    std::string data = "{vec4:color,vec3:position}";
    std::stringstream ss(data);

    std::vector<Uniform> v = Reader<std::vector<Uniform>>{}(ss);

    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0].name, expected[0].name);
    EXPECT_EQ(v[0].type, expected[0].type);
    EXPECT_EQ(v[0].arr_length, expected[0].arr_length);
    EXPECT_EQ(v[0].size, expected[0].size);

    EXPECT_EQ(v[1].name, expected[1].name);
    EXPECT_EQ(v[1].type, expected[1].type);
    EXPECT_EQ(v[1].arr_length, expected[1].arr_length);
    EXPECT_EQ(v[1].size, expected[1].size);
  }

  {
    std::string data = R"(
      {vec4:color,vec3:position}
    )";

    std::stringstream ss(data);
    std::vector<Uniform> v;
    ASSERT_NO_FATAL_FAILURE(v = Reader<std::vector<Uniform>>{}(ss));
    ASSERT_EQ(v.size(), 2u);

    EXPECT_EQ(v[0].name, "color");
    EXPECT_EQ(v[0].type, ValueType::VEC4);
    EXPECT_EQ(v[0].arr_length, 1);
    EXPECT_EQ(v[0].size, 16);

    EXPECT_EQ(v[1].name, "position");
    EXPECT_EQ(v[1].type, ValueType::VEC3);
    EXPECT_EQ(v[1].arr_length, 1);
    EXPECT_EQ(v[1].size, 12);
  }
}

TEST_F(SerializationTests, renderpass_spec) {
  {
    std::stringstream ss;
    other::RenderPassSpec spec{
      .name = "RenderPass",
      .tag_col = glm::vec4(1.f),
      .uniforms = {
        { "color", ValueType::VEC4 },
      },
      .shader = nullptr,
    };

    std::string expected_str = R"({
    RenderPass
    tag-color = {1,1,1,1}
    uniforms = {
      vec4:color
    }
    shader = 0
}
)";

    Writer<other::RenderPassSpec>{}(ss, spec);
    ASSERT_EQ(ss.str(), expected_str);
  }

  {
    std::string data = R"(
        {
          RenderPass
          tag-color = {1,1,1,1}
          uniforms = {vec4:color,vec3:position}
          shader = 0
        })";

    std::stringstream ss(data);
    other::RenderPassSpec spec;
    ASSERT_NO_FATAL_FAILURE(spec = Reader<other::RenderPassSpec>{}(ss));
    EXPECT_EQ(spec.name, "RenderPass");
    EXPECT_EQ(spec.tag_col, glm::vec4(1.f));
    ASSERT_EQ(spec.uniforms.size(), 2u);

    Uniform& uniform = spec.uniforms[0];
    ASSERT_EQ(uniform.name, "color");
    ASSERT_EQ(uniform.type, ValueType::VEC4);
    ASSERT_EQ(uniform.arr_length, 1);
    ASSERT_EQ(uniform.size, 16);

    uniform = spec.uniforms[1];
    ASSERT_EQ(uniform.name, "position");
    ASSERT_EQ(uniform.type, ValueType::VEC3);
    ASSERT_EQ(uniform.arr_length, 1);
    ASSERT_EQ(uniform.size, 12);

    ASSERT_EQ(spec.shader, nullptr);
  }
}

TEST_F(SerializationTests, pipeline_spec) {
  {
    std::stringstream ss;
    other::PipelineSpec spec{
      .topology = other::DrawMode::TRIANGLES,
      .back_face_culling = true,
      .depth_test = true,
      .line_width = 1.f,
      .framebuffer_spec = {
        .depth_func = other::DepthFunction::LESS,
        .clear_color = glm::vec4(1.f),
        .size = glm::ivec2(1920, 1080),
        .depth = true,
        .color = true,
        .stencil = false,
      },
      .pipeline_name = "Pipeline",
    };

    std::string expected_str = R"({
    Pipeline
    topology = TRIANGLES
    back-face-culling = true
    depth-test = true
    line-width = 1
    framebuffer-spec = {
    Framebuffer
    depth-function = LESS
    clear-color = {1,1,1,1}
    size = {1920,1080}
    depth-buffer = true
    color-buffer = true
    stencil-buffer = false
  }
  }
)";

    Writer<other::PipelineSpec>{}(ss, spec);
    ASSERT_EQ(ss.str(), expected_str);
  }

  {
    std::stringstream ss;
    other::PipelineSpec spec{
      .topology = other::DrawMode::TRIANGLES,
      .back_face_culling = true,
      .depth_test = true,
      .line_width = 1.f,
      .framebuffer_spec = {
        .depth_func = other::DepthFunction::LESS,
        .clear_color = glm::vec4(1.f),
        .size = glm::ivec2(1920, 1080),
        .depth = true,
        .color = true,
        .stencil = false,
      },
      .pipeline_name = "Pipeline",
    };

    std::string expected_str = R"({
    Pipeline
    topology = TRIANGLES
    back-face-culling = true
    depth-test = true
    line-width = 1
    framebuffer-spec = {
    Framebuffer
    depth-function = LESS
    clear-color = {1,1,1,1}
    size = {1920,1080}
    depth-buffer = true
    color-buffer = true
    stencil-buffer = false
  }

  }
)";

    Writer<other::PipelineSpec>{}(ss, spec);
    ASSERT_EQ(ss.str(), expected_str);
  }

  {
    std::string data = R"(
      {
        Pipeline
        topology = TRIANGLES
        back-face-culling = true
        depth-test = true
        line-width = 1
        framebuffer-spec = {
          Framebuffer
          depth-function = LESS
          clear-color = {1,1,1,1}
          size = {1920,1080}
          depth-buffer = true
          color-buffer = true
          stencil-buffer = false
        }
      }
    )";

    std::stringstream ss(data);
    other::PipelineSpec spec;
    ASSERT_NO_FATAL_FAILURE(spec = Reader<other::PipelineSpec>{}(ss));
    EXPECT_EQ(spec.pipeline_name, "Pipeline");
    EXPECT_EQ(spec.topology, other::DrawMode::TRIANGLES);
    EXPECT_EQ(spec.back_face_culling, true);
    EXPECT_EQ(spec.depth_test, true);
    EXPECT_EQ(spec.line_width, 1.f);

    EXPECT_EQ(spec.framebuffer_spec.depth_func, other::DepthFunction::LESS);
    EXPECT_EQ(spec.framebuffer_spec.clear_color, glm::vec4(1.f));
    EXPECT_EQ(spec.framebuffer_spec.size, glm::ivec2(1920, 1080));
    EXPECT_EQ(spec.framebuffer_spec.depth, true);
    EXPECT_EQ(spec.framebuffer_spec.color, true);
    EXPECT_EQ(spec.framebuffer_spec.stencil, false);
  }
}