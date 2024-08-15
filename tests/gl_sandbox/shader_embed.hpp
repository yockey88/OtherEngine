/**
 * \file shader_embed.hpp
 **/
#ifndef SHADER_EMBED_HPP
#define SHADER_EMBED_HPP

static const char* vert1 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnormal;

layout (std140 , binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
  vec4 viewpoint;
};

#define MAX_NUM_MODELS 100
layout (std430 , binding = 1) readonly buffer ModelData {
  mat4 models[MAX_NUM_MODELS];
};


out vec3 position;
out vec3 normal;
out int instanceid;

void main() {
  mat4 model = models[gl_InstanceID];
  vec4 world_pos = model * vec4(vpos , 1.0);
  position = world_pos.xyz;
  instanceid = gl_InstanceID;

  mat3 normal_mat = transpose(inverse(mat3(model)));
  normal = normal_mat * vnormal;

  gl_Position = projection * view * world_pos;
}
)";

static const char* frag1 = R"(
#version 460 core

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_spec;

in vec3 position;
in vec3 normal;
flat in int instanceid;

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};


#define MAX_NUM_MODELS 100
layout (std430 , binding = 2) readonly buffer MaterialData {
  Material materials[MAX_NUM_MODELS];
};

void main() {
  Material mat = materials[instanceid];

  g_position = position;
  g_normal = normalize(normal);

  g_albedo_spec.rgb = mat.ambient.rgb;
  g_albedo_spec.a = mat.shininess;
}
)";

static const char* vert2 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec2 vtexcoords;

out vec2 tex_coords;

void main() {
  tex_coords = vtexcoords;
  gl_Position = vec4(vpos , 1.0);
}
)";

static const char* frag2 = R"(
#version 460 core

in vec2 tex_coords;

out vec4 frag_color;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_spec;

struct PointLight {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float constant;
  float linear;
  float quadratic;
};

struct DirectionLight {
  vec4 direction;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

layout (std140 , binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
  vec4 viewpoint;
};

#define MAX_NUM_LIGHTS 100
layout (std430 , binding = 3) readonly buffer Lights {
  vec4 num_lights;
  PointLight point_lights[MAX_NUM_LIGHTS];
  DirectionLight direction_lights[MAX_NUM_LIGHTS];
};

void main() {
  vec3 frag_pos = texture(g_position , tex_coords).rgb;
  vec3 normal = texture(g_normal , tex_coords).rgb;
  vec3 albedo = texture(g_albedo_spec , tex_coords).rgb;
  float specular = texture(g_albedo_spec , tex_coords).a;

  vec3 lighting = albedo * 0.1;
  vec3 view_dir = normalize(viewpoint.xyz - frag_pos);

  for (int i = 0; i < num_lights.y; ++i) {
    // diffuse
    vec3 light_dir = normalize(point_lights[i].position.xyz - frag_pos);
    vec3 diffuse = max(dot(normal , light_dir) , 0.0) * albedo * (point_lights[i].ambient.xyz + point_lights[i].diffuse.xyz);

    // specular
    vec3 halfway = normalize(light_dir + view_dir);
    float s = pow(max(dot(normal , halfway) , 0.0) , 16.0);
    vec3 spec = point_lights[i].ambient.xyz * s * specular;

    // attenuation
    float dist = length(point_lights[i].position.xyz - frag_pos);
    float attenuation = 1.0 / (1.0 + point_lights[i].linear * dist + point_lights[i].quadratic * dist * dist);

    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + spec; 
  }
  frag_color = vec4(lighting , 1.f);
}
)";

#endif // !SHADER_EMBED_HPP
