#version 460 core

in vec4 fcolor;
in vec3 fnorm;
in vec3 ftan;
in vec3 fbitan;
in vec2 fuvs;

out vec4 FragColor;

void main() {
  FragColor = fcolor;
}
