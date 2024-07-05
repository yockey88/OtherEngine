#version 460 core

in vec3 f_position;
in vec3 f_color;
in vec3 f_normal;
in vec3 f_tangent;
in vec3 f_bitangent;
in vec2 f_uvs;

out vec4 Frag;

void main() {
  Frag = vec4(0.0 , 1.0 , 0.0 , 1.0);
}
