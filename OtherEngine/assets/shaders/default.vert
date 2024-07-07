#version 460 core
 
out vec4 fcol;

void main() {
  gl_Position = projection * view * models[model_id] * vec4(vpos , 1.0);
  fcol = vec4(vpos , 1.0);
}
