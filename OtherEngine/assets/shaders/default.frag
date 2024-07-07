#version 460 core

in vec4 fcol;
out vec4 frag_color;

void main () {
  frag_color = fcol;
}