/**
 * \file scripting/lua/lua_math_bindings.cpp
 **/
#include "scripting/lua/lua_math_bindings.hpp"
#include <sol/raii.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace other {
namespace lua_script_bindings {

  void BindGlmTypes(sol::state& lua_state) {
    auto vec2_multiply = sol::overload(
      [](const glm::vec2& v , float s) -> glm::vec2 { return s * v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s * v; }
    );

    auto vec2_divide = sol::overload(
      [](const glm::vec2& v , float s) -> glm::vec2 { return v / s; } 
    );

    auto vec2_add = sol::overload(
      [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return v1 + v2; } ,
      [](const glm::vec2& v , float s) -> glm::vec2 { return s + v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s + v; }
    );

    auto vec2_subtract = sol::overload(
      [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return v1 - v2; } ,
      [](const glm::vec2& v , float s) -> glm::vec2 { return s - v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s - v; }
    );

    auto vec3_multiply = sol::overload(
      [](const glm::vec3& v , float s) -> glm::vec3 { return s * v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s * v; }
    );

    auto vec3_divide = sol::overload(
      [](const glm::vec3& v , float s) -> glm::vec3 { return v / s; } 
    );

    auto vec3_add = sol::overload(
      [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return v1 + v2; } ,
      [](const glm::vec3& v , float s) -> glm::vec3 { return s + v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s + v; }
    );

    auto vec3_subtract = sol::overload(
      [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return v1 - v2; } ,
      [](const glm::vec3& v , float s) -> glm::vec3 { return s - v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s - v; }
    );
    
    auto vec4_multiply = sol::overload(
      [](const glm::vec4& v , float s) -> glm::vec4 { return s * v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s * v; }
    );

    auto vec4_divide = sol::overload(
      [](const glm::vec4& v , float s) -> glm::vec4 { return v / s; } 
    );

    auto vec4_add = sol::overload(
      [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return v1 + v2; } ,
      [](const glm::vec4& v , float s) -> glm::vec4 { return s + v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s + v; }
    );

    auto vec4_subtract = sol::overload(
      [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return v1 - v2; } ,
      [](const glm::vec4& v , float s) -> glm::vec4 { return s - v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s - v; }
    );
    
    lua_state.new_usertype<glm::vec2>(
      "Vec2" ,
      sol::call_constructor ,
      sol::constructors<glm::vec2(float) , glm::vec2(float , float)>() ,
      "x" , &glm::vec2::x ,
      "y" , &glm::vec2::y ,
      sol::meta_function::multiplication , vec2_multiply ,
      sol::meta_function::division , vec2_divide ,
      sol::meta_function::addition , vec2_add ,
      sol::meta_function::subtraction , vec2_subtract ,
      "length" , [](const glm::vec2& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec2& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec2& v) -> glm::vec2 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec2& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec2& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); }
    );

    lua_state.new_usertype<glm::vec3>(
      "Vec3" ,
      sol::call_constructor ,
      sol::constructors<glm::vec3(float) , glm::vec3(float , float , float)>() ,
      "x" , &glm::vec3::x ,
      "y" , &glm::vec3::y ,
      "z" , &glm::vec3::z ,
      sol::meta_function::multiplication , vec3_multiply ,
      sol::meta_function::division , vec3_divide ,
      sol::meta_function::addition , vec3_add ,
      sol::meta_function::subtraction , vec3_subtract ,
      "length" , [](const glm::vec3& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec3& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec3& v) -> glm::vec3 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_z" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.z , 0.f , std::numeric_limits<float>::epsilon()); }
    );
    
    lua_state.new_usertype<glm::vec4>(
      "Vec4" ,
      sol::call_constructor ,
      sol::constructors<glm::vec4(float) , glm::vec4(float , float , float , float)>() ,
      "x" , &glm::vec4::x ,
      "y" , &glm::vec4::y ,
      "z" , &glm::vec4::z ,
      "w" , &glm::vec4::w ,
      sol::meta_function::multiplication , vec4_multiply ,
      sol::meta_function::division , vec4_divide ,
      sol::meta_function::addition , vec4_add ,
      sol::meta_function::subtraction , vec4_subtract ,
      "length" , [](const glm::vec4& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec4& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec4& v) -> glm::vec4 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_z" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.z , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_w" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.w , 0.f , std::numeric_limits<float>::epsilon()); }
    );

    
    
    auto quat_multiply = sol::overload(
      [](const glm::quat& q1 , const glm::quat& q2) -> glm::quat { return q1 * q2; } ,
      [](float s , const glm::quat& q) -> glm::quat { return s * q; } ,
      [](const glm::quat& q , float s) -> glm::quat { return s * q; }
    );

    lua_state.new_usertype<glm::quat>(
      "Quat" , 
      sol::call_constructor , 
      sol::constructors<glm::quat(float , float , float , float) , glm::quat(float , glm::vec3)>() ,
      sol::meta_function::multiplication , quat_multiply , 
      sol::meta_function::division , [](const glm::quat& q , float s) -> glm::quat { return q / s; } ,
      sol::meta_function::addition , [](const glm::quat& q1 , const glm::quat& q2) -> glm::quat { return q1 + q2; } , 
      sol::meta_function::subtraction , [](const glm::quat& q1 , const glm::quat& q2) -> glm::quat { return q1 + q2; }
    );

    auto dot_products = sol::overload(
      [](const glm::vec2& v1 , const glm::vec2& v2) -> float { return glm::dot(v1 , v2); } , 
      [](const glm::vec3& v1 , const glm::vec3& v2) -> float { return glm::dot(v1 , v2); } , 
      [](const glm::vec4& v1 , const glm::vec4& v2) -> float { return glm::dot(v1 , v2); }
    );
    
    auto cross_products = sol::overload(
      [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1 , v2); } ,
      [](const glm::quat& q , const glm::vec3& v) -> glm::vec3 { return glm::cross(q , v); }
    );

    lua_state.set_function("dot" , dot_products);
    lua_state.set_function("cross" , cross_products);

    auto mat4_multiply = sol::overload(
      [](const glm::mat4& m1 , const glm::mat4& m2) -> glm::mat4 { return m1 * m2; } ,
      [](float s , const glm::mat4& m) -> glm::mat4 { return s * m; } ,
      [](const glm::mat4& m , float s) -> glm::mat4 { return s * m; }
    );
    
    auto mat4_divide = sol::overload(
      [](float s , const glm::mat4& m) -> glm::mat4 { return m / s; } ,
      [](const glm::mat4& m , float s) -> glm::mat4 { return m / s; }
    );

    lua_state.new_usertype<glm::mat4>(
      "Mat4" , 
      sol::call_constructor,
      sol::constructors<glm::mat4(float) , glm::mat4(glm::vec4 , glm::vec4 , glm::vec4 , glm::vec4)>() ,
      sol::meta_function::multiplication , mat4_multiply ,
      sol::meta_function::division , mat4_divide ,
      sol::meta_function::addition , [](const glm::mat4& m1 , const glm::mat4& m2) -> glm::mat4 { return m1 + m2; } ,
      sol::meta_function::subtraction , [](const glm::mat4& m1 , const glm::mat4& m2) -> glm::mat4 { return m1 - m2; } ,
      "element_at" , [](const glm::mat4& m , size_t i , size_t j) -> float { return m[i][j]; } ,
      "row" , [](const glm::mat4& m , size_t r) -> glm::vec4 { return m[r]; } ,
      "column" , [](const glm::mat4& m , size_t c) -> glm::vec4 { return glm::transpose(m)[c]; } ,
      "translate" , [](const glm::mat4& m , const glm::vec3& v) -> glm::mat4 { return glm::translate(m , v); } ,
      "scale" , [](const glm::mat4& m , const glm::vec3& v) -> glm::mat4 { return glm::scale(m , v); } ,
      "rotate" , [](const glm::mat4& m , const glm::vec3& ax , float an) -> glm::mat4 {
        return glm::rotate(m , glm::radians(an) , ax);
      } 
    );

  }

} // namespace lua_script_bindings
} // namespace other
