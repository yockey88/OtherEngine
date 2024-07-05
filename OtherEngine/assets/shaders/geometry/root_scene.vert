#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in vec2 uvs;

out vec3 f_position;
out vec3 f_color;
out vec3 f_normal;
out vec3 f_tangent;
out vec3 f_bitangent;
out vec2 f_uvs;

void main() {
  gl_Position = f_position;
  position = f_position;
  color = f_color;
  normal = f_normal;
  tangent = f_tangent;
  bitangent = f_bitangent;
  uvs = f_uvs;
}
