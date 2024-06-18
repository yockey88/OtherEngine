#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vcolor;
layout (location = 2) in vec3 vnorm;
layout (location = 3) in vec3 vtan;
layout (location = 4) in vec3 vbitan;
layout (location = 5) in vec2 vuvs;

out vec4 fcolor;
out vec3 fnorm;
out vec3 ftan;
out vec3 fbitan;
out vec2 fuvs;

uniform mat4 ucamera;

void main() {
  gl_Position = ucamera * vec4(vpos , 1.0);
  fcolor = vec4(vcolor , 1.0);
  fnorm = vnorm;
  ftan = vtan;
  fbitan = vbitan;
  fuvs = vuvs;
}
