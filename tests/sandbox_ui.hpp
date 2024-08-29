/**
 * \file sandbox/sandbox_ui.hpp
 **/
#ifndef OTHER_ENGINE_SANDBOX_UI_HPP
#define OTHER_ENGINE_SANDBOX_UI_HPP

#include <string>

#include <imgui/imgui.h>

#include "rendering/material.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"

void RenderItem(uint32_t id , const std::string& title , ImVec2 size = ImVec2(800 , 600));
bool RenderMaterial(const std::string& title , other::Material& mat);
bool RenderPointLight(const std::string& title , other::PointLight& pl);
bool RenderDirectionLight(const std::string& title , other::DirectionLight& pl);

void RenderSettings();

#endif // !OTHER_ENGINE_SANDBOX_UI_HPP
