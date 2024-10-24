/**
 * \file scripting/lua/lua_ui_bindings.cpp
 **/
#include "scripting/lua/lua_ui_bindings.hpp"

#include <string_view>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/defines.hpp"

#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

namespace other {
  namespace lua_script_bindings {
    namespace {

      /// Core Functions

      static bool Begin(const std::string& name) {
        return ImGui::Begin(name.c_str());
      }

      static std::tuple<bool, bool> Begin(const std::string& name, bool is_open) {
        if (!is_open) {
          return std::make_tuple(false, false);
        }

        bool can_draw = ImGui::Begin(name.c_str(), &is_open);
        if (!is_open) {
          ImGui::End();
          return std::make_tuple(false, false);
        }

        return std::make_tuple(is_open, can_draw);
      }

      static std::tuple<bool, bool> Begin(const std::string& name, bool is_open, int32_t flags) {
        if (!is_open) {
          return std::make_tuple(false, false);
        }

        bool can_draw = ImGui::Begin(name.c_str(), &is_open, static_cast<ImGuiWindowFlags_>(flags));
        if (!is_open) {
          ImGui::End();
          return std::make_tuple(false, false);
        }

        return std::make_tuple(is_open, can_draw);
      }

      static void End() {
        ImGui::End();
      }

      static void PushId(int32_t id) {
        ImGui::PushID(id);
      }

      static void PopId() {
        ImGui::PopID();
      }

      static int32_t GetId(const std::string& str_id) {
        return ImGui::GetID(str_id.c_str());
      }

      static void PushItemFlag(int32_t flag, bool enabled) {
        ImGui::PushItemFlag(static_cast<ImGuiItemFlags_>(flag), enabled);
      }

      static void PopItemFlag() {
        ImGui::PopItemFlag();
      }

      static void PushFont(ImFont* pFont) { ImGui::PushFont(pFont); }
      static void PopFont() { ImGui::PopFont(); }

      static void PushStyleColor(int32_t idx, int32_t col) { ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), ImU32(col)); }

      static void PushStyleColor(int32_t idx, float colR, float colG, float colB, float colA) {
        ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), { colR, colG, colB, colA });
      }

      static void PopStyleColor() { ImGui::PopStyleColor(); }

      static void PopStyleColor(int32_t count) {
        ImGui::PopStyleColor(count);
      }
      static void PushStyleVar(int32_t idx, float val) {
        ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), val);
      }
      static void PushStyleVar(int32_t idx, float valX, float valY) {
        ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), { valX, valY });
      }
      static void PopStyleVar() {
        ImGui::PopStyleVar();
      }
      static void PopStyleVar(int32_t count) {
        ImGui::PopStyleVar(count);
      }
      static std::tuple<float, float, float, float> GetStyleColorVec4(int32_t idx) {
        const auto col{ ImGui::GetStyleColorVec4(static_cast<ImGuiCol>(idx)) };
        return std::make_tuple(col.x, col.y, col.z, col.w);
      }
      static ImFont* GetFont() {
        return ImGui::GetFont();
      }

      static float GetFontSize() { return ImGui::GetFontSize(); }

      static std::tuple<float, float> GetFontTexUvWhitePixel() {
        const auto vec2{ ImGui::GetFontTexUvWhitePixel() };
        return std::make_tuple(vec2.x, vec2.y);
      }
      static int32_t GetColorU32(int32_t idx, float alphaMul) { return ImGui::GetColorU32(static_cast<ImGuiCol>(idx), alphaMul); }
      static int32_t GetColorU32(float colR, float colG, float colB, float colA) { return ImGui::GetColorU32({ colR, colG, colB, colA }); }
      static int32_t GetColorU32(int32_t col) { return ImGui::GetColorU32(ImU32(col)); }

      /// SetNext Section

      static void SetNextWindowSize(float size_x, float size_y) {
        ImGui::SetNextWindowSize({ size_x, size_y });
      }

      static void SetNextWindowSize(float size_x, float size_y, int32_t cond) {
        ImGui::SetNextWindowSize({ size_x, size_y }, static_cast<ImGuiCond_>(cond));
      }

      static void SetNextWindowSizeConstraints(float min_x, float min_y, float max_x, float max_y) {
        ImGui::SetNextWindowSizeConstraints({ min_x, min_y }, { max_x, max_y });
      }

      static void SetNextItemWidth(float width) {
        ImGui::SetNextItemWidth(width);
      }

      /// Get Section

      static std::tuple<float, float> GetContentRegionMax() {
        const auto vec2{ ImGui::GetContentRegionMax() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static std::tuple<float, float> GetContentRegionAvail() {
        const auto vec2{ ImGui::GetContentRegionAvail() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static std::tuple<float, float> GetWindowContentRegionMin() {
        const auto vec2{ ImGui::GetWindowContentRegionMin() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static std::tuple<float, float> GetWindowContentRegionMax() {
        const auto vec2{ ImGui::GetWindowContentRegionMax() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      /// Text Section

      static void Text(const std::string& text) {
        ImGui::Text("%s", text.c_str());
      }

      /// Clickable Section

      static bool Button(const std::string& label) {
        return ImGui::Button(label.c_str());
      }

      static bool Button(const std::string& label, float size_x, float size_y) {
        return ImGui::Button(label.c_str(), { size_x, size_y });
      }

      static std::tuple<float, bool> DragFloat(const std::string& label, float v) {
        bool used = ImGui::DragFloat(label.c_str(), &v);
        return std::make_tuple(v, used);
      }
      static std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed) {
        bool used = ImGui::DragFloat(label.c_str(), &v, v_speed);
        return std::make_tuple(v, used);
      }
      static std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min) {
        bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min);
        return std::make_tuple(v, used);
      }
      static std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max) {
        bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max);
        return std::make_tuple(v, used);
      }
      static std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max, const std::string& format) {
        bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str());
        return std::make_tuple(v, used);
      }
      static std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max, const std::string& format, float power) {
        bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str(), static_cast<int>(power));
        return std::make_tuple(v, used);
      }

      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorEdit3(label.c_str(), color);

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
          color[0], color[1], color[2] });

        return std::make_tuple(rgb, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col, int flags) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorEdit3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
          color[0], color[1], color[2] });

        return std::make_tuple(rgb, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorEdit4(label.c_str(), color);

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
          color[0], color[1], color[2], color[3] });

        return std::make_tuple(rgba, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col, int flags) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorEdit4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
          color[0], color[1], color[2], color[3] });

        return std::make_tuple(rgba, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorPicker3(label.c_str(), color);

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
          color[0], color[1], color[2] });

        return std::make_tuple(rgb, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col, int flags) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorPicker3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
          color[0], color[1], color[2] });

        return std::make_tuple(rgb, used);
      }

      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorPicker4(label.c_str(), color);

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
          color[0], color[1], color[2], color[3] });

        return std::make_tuple(rgba, used);
      }
      static std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col, int flags) {
        const lua_Number r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorPicker4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
          color[0], color[1], color[2], color[3] });

        return std::make_tuple(rgba, used);
      }

      /// Table Section

      static void TableSetColumnIndext(int32_t n) {
        ImGui::TableSetColumnIndex(n);
      }

      /// Layout Section

      static void BeginVertical(const std::string& name, const glm::vec2& size, float align) {
        ImGui::BeginVertical(name.c_str(), ImVec2{ size.x, size.y }, align);
      }

      static void EndVertical() {
        ImGui::EndVertical();
      }

      static void Separator() {
        ImGui::Separator();
      }

      static void SameLine() {
        ImGui::SameLine();
      }

      static void SameLine(float offsetFromStartX) {
        ImGui::SameLine(offsetFromStartX);
      }

      static void SameLine(float offsetFromStartX, float spacing) {
        ImGui::SameLine(offsetFromStartX, spacing);
      }

      static void NewLine() {
        ImGui::NewLine();
      }

      static void Spacing() {
        ImGui::Spacing();
      }

      static void Dummy(float sizeX, float sizeY) {
        ImGui::Dummy({ sizeX, sizeY });
      }

      static void Indent() {
        ImGui::Indent();
      }

      static void Indent(float indentW) {
        ImGui::Indent(indentW);
      }

      static void Unindent() {
        ImGui::Unindent();
      }

      static void Unindent(float indentW) {
        ImGui::Unindent(indentW);
      }

      static void BeginGroup() {
        ImGui::BeginGroup();
      }

      static void EndGroup() {
        ImGui::EndGroup();
      }

      static std::tuple<float, float> GetCursorPos() {
        const auto vec2{ ImGui::GetCursorPos() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static float GetCursorPosX() {
        return ImGui::GetCursorPosX();
      }

      static float GetCursorPosY() {
        return ImGui::GetCursorPosY();
      }

      static void SetCursorPos(float localX, float localY) {
        ImGui::SetCursorPos({ localX, localY });
      }

      static void SetCursorPosX(float localX) {
        ImGui::SetCursorPosX(localX);
      }

      static void SetCursorPosY(float localY) {
        ImGui::SetCursorPosY(localY);
      }

      static std::tuple<float, float> GetCursorStartPos() {
        const auto vec2{ ImGui::GetCursorStartPos() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static std::tuple<float, float> GetCursorScreenPos() {
        const auto vec2{ ImGui::GetCursorScreenPos() };
        return std::make_tuple(vec2.x, vec2.y);
      }

      static void SetCursorScreenPos(float posX, float posY) {
        ImGui::SetCursorScreenPos({ posX, posY });
      }

      static void AlignTextToFramePadding() {
        ImGui::AlignTextToFramePadding();
      }

      static float GetTextLineHeight() {
        return ImGui::GetTextLineHeight();
      }

      static float GetTextLineHeightWithSpacing() {
        return ImGui::GetTextLineHeightWithSpacing();
      }

      static float GetFrameHeight() {
        return ImGui::GetFrameHeight();
      }

      static float GetFrameHeightWithSpacing() {
        return ImGui::GetFrameHeightWithSpacing();
      }

      /// Item Interaction Section

      static bool TempInputIsActive(int32_t id) {
        return ImGui::TempInputIsActive(static_cast<ImGuiID>(id));
      }

      static bool IsItemDeactivatedAfterEdit() {
        return ImGui::IsItemDeactivatedAfterEdit();
      }

      /// Engine UI Widgets

      static bool DrawVec2Control(const std::string_view label, glm::vec2& value, float reset_val, float col_w, int32_t axes, const glm::vec2& v_min, const glm::vec2& v_max, float speed) {
        bool edited = false;
        return ui::widgets::DrawVec2Control(label, value, edited, reset_val, col_w, static_cast<ui::VectorAxis>(axes), v_min, v_max, speed) || edited;
      }

      static bool EditVec2(const std::string_view label, ImVec2 size, float reset_val, glm::vec2& value, int32_t axes, float speed, const glm::vec2& v_min, const glm::vec2& v_max, const char* format, int32_t flags) {
        bool edited = false;
        return ui::widgets::EditVec2(label, size, reset_val, edited, value, static_cast<ui::VectorAxis>(axes), speed, v_min, v_max, format, flags) || edited;
      }

      static bool DrawVec3Control(const std::string_view label, glm::vec3& value, float reset_value, float col_w, int32_t axes, const glm::vec3& min, const glm::vec3& max, float speed) {  // }, float reset_val, float col_w, int32_t axes, const glm::vec3& v_min, const glm::vec3& v_max, float speed) {
        bool edited = false;
        return ui::widgets::DrawVec3Control(label, value, edited, reset_value, col_w, static_cast<ui::VectorAxis>(axes), min, max, speed) || edited;
      }

      static bool EditVec3(const std::string_view label, ImVec2 size, float reset_val, glm::vec3& value, int32_t axes, float speed, const glm::vec3& v_min, const glm::vec3& v_max, const char* format, int32_t flags) {
        bool edited = false;
        return ui::widgets::EditVec3(label, size, reset_val, edited, value, static_cast<ui::VectorAxis>(axes), speed, v_min, v_max, format, flags) || edited;
      }

      static bool DrawVec4Control(const std::string_view label, glm::vec4& value, float reset_val, float col_w, int32_t axes, const glm::vec4& v_min, const glm::vec4& v_max, float speed) {
        bool edited = false;
        return ui::widgets::DrawVec4Control(label, value, edited, reset_val, col_w, static_cast<ui::VectorAxis>(axes), v_min, v_max, speed) || edited;
      }

      static bool EditVec4(const std::string_view label, ImVec2 size, float reset_val, glm::vec4& value, int32_t axes, float speed, const glm::vec4& v_min, const glm::vec4& v_max, const char* format, int32_t flags) {
        bool edited = false;
        return ui::widgets::EditVec4(label, size, reset_val, edited, value, static_cast<ui::VectorAxis>(axes), speed, v_min, v_max, format, flags) || edited;
      }

      static int64_t ui_item_id = 0;

      static void PushRandomID() {
        ImGui::PushID(ui_item_id++);
      }

      static void PopRandomID() {
        ImGui::PopID();
        if (ui_item_id > 0) {
          ui_item_id--;
        }
      }

    }  // anonymous namespace

    void BindUiTypes(sol::state& lua_state) {
      lua_state.new_enum(
        "ImGuiWindowFlags",
        "None", ImGuiWindowFlags_None,
        "NoTitleBar", ImGuiWindowFlags_NoTitleBar,
        "NoResize", ImGuiWindowFlags_NoResize,
        "NoMove", ImGuiWindowFlags_NoMove,
        "NoScrollbar", ImGuiWindowFlags_NoScrollbar,
        "NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse,
        "NoCollapse", ImGuiWindowFlags_NoCollapse,
        "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize,
        "NoBackground", ImGuiWindowFlags_NoBackground,
        "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings,
        "NoMouseInputs", ImGuiWindowFlags_NoMouseInputs,
        "MenuBar", ImGuiWindowFlags_MenuBar,
        "HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar,
        "NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing,
        "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus,
        "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar,
        "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar,
        "AlwaysUseWindowPadding", ImGuiWindowFlags_AlwaysUseWindowPadding,
        "NoNavInputs", ImGuiWindowFlags_NoNavInputs,
        "NoNavFocus", ImGuiWindowFlags_NoNavFocus,
        "UnsavedDocument", ImGuiWindowFlags_UnsavedDocument,
        "NoDocking", ImGuiWindowFlags_NoDocking,
        "NoNav", ImGuiWindowFlags_NoNav,
        "NoDecoration", ImGuiWindowFlags_NoDecoration,
        "NoInputs", ImGuiWindowFlags_NoInputs,
        "NavFlattened", ImGuiWindowFlags_NavFlattened,
        "ChildWindow", ImGuiWindowFlags_ChildWindow,
        "Tooltip", ImGuiWindowFlags_Tooltip,
        "Popup", ImGuiWindowFlags_Popup,
        "Modal", ImGuiWindowFlags_Modal,
        "ChildMenu", ImGuiWindowFlags_ChildMenu,
        "DockNodeHost", ImGuiWindowFlags_DockNodeHost,
        // "NoClose", ImGuiWindowFlags_NoClose,
        // "NoDockTab", ImGuiWindowFlags_NoDockTab,
        "DockNodeHost", ImGuiWindowFlags_DockNodeHost
      );

      lua_state.new_enum(
        "ImGuiCond",
        "None", ImGuiCond_None,
        "Always", ImGuiCond_Always,
        "Once", ImGuiCond_Once,
        "FirstUseEver", ImGuiCond_FirstUseEver,
        "Appearing", ImGuiCond_Appearing
      );

      lua_state.new_enum(
        "ImGuiItemFlags",
        "None", ImGuiItemFlags_None,
        "NoTabStop", ImGuiItemFlags_NoTabStop,
        "ButtonRepeat", ImGuiItemFlags_ButtonRepeat,
        "Disabled", ImGuiItemFlags_Disabled,
        "NoNav", ImGuiItemFlags_NoNav,
        "NoNavDefaultFocus", ImGuiItemFlags_NoNavDefaultFocus,
        "SelectableDontClosePopup", ImGuiItemFlags_SelectableDontClosePopup,
        "MixedValue", ImGuiItemFlags_MixedValue,
        "ReadOnly", ImGuiItemFlags_ReadOnly,
        "NoWindowHoverableCheck", ImGuiItemFlags_NoWindowHoverableCheck,
        "AllowOverlap", ImGuiItemFlags_AllowOverlap,
        "Inputable", ImGuiItemFlags_Inputable,
        "HasSelectionUserData", ImGuiItemFlags_HasSelectionUserData
      );

      lua_state.new_enum(
        "ImGuiSliderFlags",
        "None", ImGuiSliderFlags_None,
        "AlwaysClamp", ImGuiSliderFlags_AlwaysClamp,
        "Logarithmic", ImGuiSliderFlags_Logarithmic,
        "NoRoundToFormat", ImGuiSliderFlags_NoRoundToFormat,
        "NoInput", ImGuiSliderFlags_NoInput
      );

      lua_state.new_enum(
        "ImGuiFocusedFlags",
        "None", ImGuiFocusedFlags_None,
        "ChildWindows", ImGuiFocusedFlags_ChildWindows,
        "RootWindow", ImGuiFocusedFlags_RootWindow,
        "AnyWindow", ImGuiFocusedFlags_AnyWindow,
        "RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows
      );

      lua_state.new_enum(
        "ImGuiHoveredFlags",
        "None", ImGuiHoveredFlags_None,
        "ChildWindows", ImGuiHoveredFlags_ChildWindows,
        "RootWindow", ImGuiHoveredFlags_RootWindow,
        "AnyWindow", ImGuiHoveredFlags_AnyWindow,
        "AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup,
        "AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem,
        "AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped,
        "AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled,
        "RectOnly", ImGuiHoveredFlags_RectOnly,
        "RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows
      );

      lua_state.new_enum(
        "ImGuiCol",
        "Text", ImGuiCol_Text,
        "TextDisabled", ImGuiCol_TextDisabled,
        "WindowBg", ImGuiCol_WindowBg,
        "ChildBg", ImGuiCol_ChildBg,
        "PopupBg", ImGuiCol_PopupBg,
        "Border", ImGuiCol_Border,
        "BorderShadow", ImGuiCol_BorderShadow,
        "FrameBg", ImGuiCol_FrameBg,
        "FrameBgHovered", ImGuiCol_FrameBgHovered,
        "FrameBgActive", ImGuiCol_FrameBgActive,
        "TitleBg", ImGuiCol_TitleBg,
        "TitleBgActive", ImGuiCol_TitleBgActive,
        "TitleBgCollapsed", ImGuiCol_TitleBgCollapsed,
        "MenuBarBg", ImGuiCol_MenuBarBg,
        "ScrollbarBg", ImGuiCol_ScrollbarBg,
        "ScrollbarGrab", ImGuiCol_ScrollbarGrab,
        "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered,
        "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive,
        "CheckMark", ImGuiCol_CheckMark,
        "SliderGrab", ImGuiCol_SliderGrab,
        "SliderGrabActive", ImGuiCol_SliderGrabActive,
        "Button", ImGuiCol_Button,
        "ButtonHovered", ImGuiCol_ButtonHovered,
        "ButtonActive", ImGuiCol_ButtonActive,
        "Header", ImGuiCol_Header,
        "HeaderHovered", ImGuiCol_HeaderHovered,
        "HeaderActive", ImGuiCol_HeaderActive,
        "Separator", ImGuiCol_Separator,
        "SeparatorHovered", ImGuiCol_SeparatorHovered,
        "SeparatorActive", ImGuiCol_SeparatorActive,
        "ResizeGrip", ImGuiCol_ResizeGrip,
        "ResizeGripHovered", ImGuiCol_ResizeGripHovered,
        "ResizeGripActive", ImGuiCol_ResizeGripActive,
        "Tab", ImGuiCol_Tab,
        "TabHovered", ImGuiCol_TabHovered,
        "TabActive", ImGuiCol_TabActive,
        "TabUnfocused", ImGuiCol_TabUnfocused,
        "TabUnfocusedActive", ImGuiCol_TabUnfocusedActive,
        "DockingPreview", ImGuiCol_DockingPreview,
        "DockingEmptyBg", ImGuiCol_DockingEmptyBg,
        "PlotLines", ImGuiCol_PlotLines,
        "PlotLinesHovered", ImGuiCol_PlotLinesHovered,
        "PlotHistogram", ImGuiCol_PlotHistogram,
        "PlotHistogramHovered", ImGuiCol_PlotHistogramHovered,
        "TextSelectedBg", ImGuiCol_TextSelectedBg,
        "DragDropTarget", ImGuiCol_DragDropTarget,
        "NavHighlight", ImGuiCol_NavHighlight,
        "NavWindowingHighlight", ImGuiCol_NavWindowingHighlight,
        "NavWindowingDimBg", ImGuiCol_NavWindowingDimBg,
        "ModalWindowDimBg", ImGuiCol_ModalWindowDimBg,
        "ModalWindowDarkening", ImGuiCol_ModalWindowDimBg,
        "COUNT", ImGuiCol_COUNT
      );

      lua_state.new_enum(
        "ImGuiStyleVar",
        "Alpha", ImGuiStyleVar_Alpha,
        "WindowPadding", ImGuiStyleVar_WindowPadding,
        "WindowRounding", ImGuiStyleVar_WindowRounding,
        "WindowBorderSize", ImGuiStyleVar_WindowBorderSize,
        "WindowMinSize", ImGuiStyleVar_WindowMinSize,
        "WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign,
        "ChildRounding", ImGuiStyleVar_ChildRounding,
        "ChildBorderSize", ImGuiStyleVar_ChildBorderSize,
        "PopupRounding", ImGuiStyleVar_PopupRounding,
        "PopupBorderSize", ImGuiStyleVar_PopupBorderSize,
        "FramePadding", ImGuiStyleVar_FramePadding,
        "FrameRounding", ImGuiStyleVar_FrameRounding,
        "FrameBorderSize", ImGuiStyleVar_FrameBorderSize,
        "ItemSpacing", ImGuiStyleVar_ItemSpacing,
        "ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing,
        "IndentSpacing", ImGuiStyleVar_IndentSpacing,
        "ScrollbarSize", ImGuiStyleVar_ScrollbarSize,
        "ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding,
        "GrabMinSize", ImGuiStyleVar_GrabMinSize,
        "GrabRounding", ImGuiStyleVar_GrabRounding,
        "TabRounding", ImGuiStyleVar_TabRounding,
        // "SelectableRounding", ImGuiStyleVar_SelectableRounding,
        "SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign,
        "ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign,
        "COUNT", ImGuiStyleVar_COUNT
      );

      lua_state.new_enum(
        "ImGuiDir",
        "None", ImGuiDir_None,
        "Left", ImGuiDir_Left,
        "Right", ImGuiDir_Right,
        "Up", ImGuiDir_Up,
        "Down", ImGuiDir_Down,
        "COUNT", ImGuiDir_COUNT
      );

      lua_state.new_enum(
        "ImGuiComboFlags",
        "None", ImGuiComboFlags_None,
        "PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft,
        "HeightSmall", ImGuiComboFlags_HeightSmall,
        "HeightRegular", ImGuiComboFlags_HeightRegular,
        "HeightLarge", ImGuiComboFlags_HeightLarge,
        "HeightLargest", ImGuiComboFlags_HeightLargest,
        "NoArrowButton", ImGuiComboFlags_NoArrowButton,
        "NoPreview", ImGuiComboFlags_NoPreview,
        "HeightMask", ImGuiComboFlags_HeightMask_
      );

      lua_state.new_enum(
        "ImGuiInputTextFlags",
        "None", ImGuiInputTextFlags_None,
        "CharsDecimal", ImGuiInputTextFlags_CharsDecimal,
        "CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal,
        "CharsUppercase", ImGuiInputTextFlags_CharsUppercase,
        "CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank,
        "AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll,
        "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue,
        "CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion,
        "CallbackHistory", ImGuiInputTextFlags_CallbackHistory,
        "CallbackAlways", ImGuiInputTextFlags_CallbackAlways,
        "CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter,
        "AllowTabInput", ImGuiInputTextFlags_AllowTabInput,
        "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine,
        "NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll,
        // "AlwaysInsertMode", ImGuiInputTextFlags_AlwaysInsertMode,
        "ReadOnly", ImGuiInputTextFlags_ReadOnly,
        "Password", ImGuiInputTextFlags_Password,
        "NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo,
        "CharsScientific", ImGuiInputTextFlags_CharsScientific,
        "CallbackResize", ImGuiInputTextFlags_CallbackResize,
        "Multiline", ImGuiInputTextFlags_Multiline,
        "NoMarkEdited", ImGuiInputTextFlags_NoMarkEdited
      );

      lua_state.new_enum(
        "ImGuiColorEditFlags",
        "None", ImGuiColorEditFlags_None,
        "NoAlpha", ImGuiColorEditFlags_NoAlpha,
        "NoPicker", ImGuiColorEditFlags_NoPicker,
        "NoOptions", ImGuiColorEditFlags_NoOptions,
        "NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview,
        "NoInputs", ImGuiColorEditFlags_NoInputs,
        "NoTooltip", ImGuiColorEditFlags_NoTooltip,
        "NoLabel", ImGuiColorEditFlags_NoLabel,
        "NoSidePreview", ImGuiColorEditFlags_NoSidePreview,
        "NoDragDrop", ImGuiColorEditFlags_NoDragDrop,
        "NoBorder", ImGuiColorEditFlags_NoBorder,
        "AlphaBar", ImGuiColorEditFlags_AlphaBar,
        "AlphaPreview", ImGuiColorEditFlags_AlphaPreview,
        "AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf,
        "HDR", ImGuiColorEditFlags_HDR,
        "DisplayRGB", ImGuiColorEditFlags_DisplayRGB,
        "DisplayHSV", ImGuiColorEditFlags_DisplayHSV,
        "DisplayHex", ImGuiColorEditFlags_DisplayHex,
        "Uint8", ImGuiColorEditFlags_Uint8,
        "Float", ImGuiColorEditFlags_Float,
        "PickerHueBar", ImGuiColorEditFlags_PickerHueBar,
        "PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel,
        "InputRGB", ImGuiColorEditFlags_InputRGB,
        "InputHSV", ImGuiColorEditFlags_InputHSV

        // "_OptionsDefault", ImGuiColorEditFlags__OptionsDefault,

        // "_DisplayMask", ImGuiColorEditFlags__DisplayMask,
        // "_DataTypeMask", ImGuiColorEditFlags__DataTypeMask,
        // "_PickerMask", ImGuiColorEditFlags__PickerMask,
        // "_InputMask", ImGuiColorEditFlags__InputMask,

        // "RGB", ImGuiColorEditFlags_RGB
      );

      lua_state.new_enum(
        "ImGuiTreeNodeFlags",
        "None", ImGuiTreeNodeFlags_None,
        "Selected", ImGuiTreeNodeFlags_Selected,
        "Framed", ImGuiTreeNodeFlags_Framed,
        "AllowItemOverlap", ImGuiTreeNodeFlags_AllowItemOverlap,
        "NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen,
        "NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog,
        "DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen,
        "OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick,
        "OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow,
        "Leaf", ImGuiTreeNodeFlags_Leaf,
        "Bullet", ImGuiTreeNodeFlags_Bullet,
        "FramePadding", ImGuiTreeNodeFlags_FramePadding,
        "SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth,
        "SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth,
        "NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere,
        "CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader
      );

      lua_state.new_enum("ImGuiSelectableFlags", "None", ImGuiSelectableFlags_None, "DontClosePopups", ImGuiSelectableFlags_DontClosePopups, "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns, "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick, "Disabled", ImGuiSelectableFlags_Disabled, "AllowItemOverlap", ImGuiSelectableFlags_AllowItemOverlap);

      lua_state.new_enum("ImGuiPopupFlags", "None", ImGuiPopupFlags_None, "MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft, "MouseButtonRight", ImGuiPopupFlags_MouseButtonRight, "MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle, "MouseButtonMask_", ImGuiPopupFlags_MouseButtonMask_, "MouseButtonDefault_", ImGuiPopupFlags_MouseButtonDefault_, "NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup, "NoOpenOverItems", ImGuiPopupFlags_NoOpenOverItems, "AnyPopupId", ImGuiPopupFlags_AnyPopupId, "AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel, "AnyPopup", ImGuiPopupFlags_AnyPopup);

      lua_state.new_enum("ImGuiTabBarFlags", "None", ImGuiTabBarFlags_None, "Reorderable", ImGuiTabBarFlags_Reorderable, "AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs, "TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton, "NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton, "NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons, "NoTooltip", ImGuiTabBarFlags_NoTooltip, "FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown, "FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll, "FittingPolicyMask_", ImGuiTabBarFlags_FittingPolicyMask_, "FittingPolicyDefault_", ImGuiTabBarFlags_FittingPolicyDefault_);

      lua_state.new_enum("ImGuiTabItemFlags", "None", ImGuiTabItemFlags_None, "UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument, "SetSelected", ImGuiTabItemFlags_SetSelected, "NoCloseWithMiddleMouseButton", ImGuiTabItemFlags_NoCloseWithMiddleMouseButton, "NoPushId", ImGuiTabItemFlags_NoPushId, "NoTooltip", ImGuiTabItemFlags_NoTooltip);

      // #pragma region DockNode Flags
      //   lua_state.new_enum("ImGuiDockNodeFlags",
      //     "None", ImGuiDockNodeFlags_None,
      //     "KeepAliveOnly", ImGuiDockNodeFlags_KeepAliveOnly,
      //     "NoDockingInCentralNode", ImGuiDockNodeFlags_NoDockingInCentralNode,
      //     "PassthruCentralNode", ImGuiDockNodeFlags_PassthruCentralNode,
      //     "NoSplit", ImGuiDockNodeFlags_NoSplit,
      //     "NoResize", ImGuiDockNodeFlags_NoResize,
      //     "AutoHideTabBar", ImGuiDockNodeFlags_AutoHideTabBar
      //   );
      // #pragma endregion DockNode Flags

      lua_state.new_enum("ImGuiMouseButton", "ImGuiMouseButton_Left", ImGuiMouseButton_Left, "ImGuiMouseButton_Right", ImGuiMouseButton_Right, "ImGuiMouseButton_Middle", ImGuiMouseButton_Middle, "ImGuiMouseButton_COUNT", ImGuiMouseButton_COUNT);
      lua_state.new_enum("ImGuiKey", "Tab", ImGuiKey_Tab, "LeftArrow", ImGuiKey_LeftArrow, "RightArrow", ImGuiKey_RightArrow, "UpArrow", ImGuiKey_UpArrow, "DownArrow", ImGuiKey_DownArrow, "PageUp", ImGuiKey_PageUp, "PageDown", ImGuiKey_PageDown, "Home", ImGuiKey_Home, "End", ImGuiKey_End, "Insert", ImGuiKey_Insert, "Delete", ImGuiKey_Delete, "Backspace", ImGuiKey_Backspace, "Space", ImGuiKey_Space, "Enter", ImGuiKey_Enter, "Escape", ImGuiKey_Escape, "KeyPadEnter", ImGuiKey_KeypadEnter, "A", ImGuiKey_A, "C", ImGuiKey_C, "V", ImGuiKey_V, "X", ImGuiKey_X, "Y", ImGuiKey_Y, "Z", ImGuiKey_Z, "COUNT", ImGuiKey_COUNT);
      lua_state.new_enum("ImGuiMouseCursor", "None", ImGuiMouseCursor_None, "Arrow", ImGuiMouseCursor_Arrow, "TextInput", ImGuiMouseCursor_TextInput, "ResizeAll", ImGuiMouseCursor_ResizeAll, "ResizeNS", ImGuiMouseCursor_ResizeNS, "ResizeEW", ImGuiMouseCursor_ResizeEW, "ResizeNESW", ImGuiMouseCursor_ResizeNESW, "ResizeNWSE", ImGuiMouseCursor_ResizeNWSE, "Hand", ImGuiMouseCursor_Hand, "NotAllowed", ImGuiMouseCursor_NotAllowed, "COUNT", ImGuiMouseCursor_COUNT);

      lua_state.new_enum(
        "VectorAxis",
        "ZERO", ui::VectorAxis::ZERO,
        "X", ui::VectorAxis::X,
        "Y", ui::VectorAxis::Y,
        "Z", ui::VectorAxis::Z,
        "W", ui::VectorAxis::W
      );

      /// Core Functions

      sol::table ImGui_bindings = lua_state.create_named_table("ImGui");
      ImGui_bindings.set_function(
        "Begin",
        sol::overload(
          sol::resolve<bool(const std::string&)>(Begin),
          sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(Begin),
          sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int32_t)>(Begin)
        )
      );
      ImGui_bindings.set_function("End", End);

      ImGui_bindings.set_function("PushId", PushId);
      ImGui_bindings.set_function("PopId", PopId);
      ImGui_bindings.set_function("GetId", GetId);

      ImGui_bindings.set_function("PushItemFlag", PushItemFlag);
      ImGui_bindings.set_function("PopItemFlag", PopItemFlag);

      ImGui_bindings.set_function("PushFont", PushFont);
      ImGui_bindings.set_function("PopFont", PopFont);
      ImGui_bindings.set_function(
        "PushStyleColor",
        sol::overload(
          sol::resolve<void(int, int)>(PushStyleColor),
          sol::resolve<void(int, float, float, float, float)>(PushStyleColor)
        )
      );
      ImGui_bindings.set_function(
        "PopStyleColor",
        sol::overload(
          sol::resolve<void()>(PopStyleColor),
          sol::resolve<void(int)>(PopStyleColor)
        )
      );
      ImGui_bindings.set_function("GetStyleColorVec4", GetStyleColorVec4);
      ImGui_bindings.set_function("GetFont", GetFont);
      ImGui_bindings.set_function("GetFontSize", GetFontSize);
      ImGui_bindings.set_function("GetFontTexUvWhitePixel", GetFontTexUvWhitePixel);
      ImGui_bindings.set_function(
        "GetColorU32",
        sol::overload(
          sol::resolve<int(int, float)>(GetColorU32),
          sol::resolve<int(float, float, float, float)>(GetColorU32),
          sol::resolve<int(int)>(GetColorU32)
        )
      );

      /// SetNext Section

      ImGui_bindings.set_function(
        "SetNextWindowSize",
        sol::overload(
          sol::resolve<void(float, float)>(SetNextWindowSize),
          sol::resolve<void(float, float, int)>(SetNextWindowSize)
        )
      );
      ImGui_bindings.set_function("SetNextWindowSizeConstraints", SetNextWindowSizeConstraints);
      ImGui_bindings.set_function("SetNextItemWidth", SetNextItemWidth);

      /// Get Section

      ImGui_bindings.set_function("GetContentRegionMax", GetContentRegionMax);
      ImGui_bindings.set_function("GetContentRegionAvail", GetContentRegionAvail);
      ImGui_bindings.set_function("GetWindowContentRegionMin", GetWindowContentRegionMin);
      ImGui_bindings.set_function("GetWindowContentRegionMax", GetWindowContentRegionMax);

      /// Text Section

      ImGui_bindings.set_function("Text", Text);

      /// Clickable Section

      ImGui_bindings.set_function(
        "Button",
        sol::overload(
          sol::resolve<bool(const std::string&)>(Button),
          sol::resolve<bool(const std::string&, float, float)>(Button)
        )
      );

      ImGui_bindings.set_function(
        "DragFloat",
        sol::overload(
          sol::resolve<std::tuple<float, bool>(const std::string&, float)>(DragFloat),
          sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(DragFloat),
          sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(DragFloat),
          sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float)>(DragFloat),
          sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&)>(DragFloat),
          sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&, float)>(DragFloat)
        )
      );

      ImGui_bindings.set_function(
        "ColorEdit3",
        sol::overload(
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorEdit3),
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorEdit3)
        )
      );
      ImGui_bindings.set_function(
        "ColorEdit4",
        sol::overload(
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorEdit4),
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorEdit4)
        )
      );

      ImGui_bindings.set_function(
        "ColorPicker3",
        sol::overload(
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorPicker3),
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorPicker3)
        )
      );

      ImGui_bindings.set_function(
        "ColorPicker4",
        sol::overload(
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorPicker4),
          sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorPicker4)
        )
      );

      /// Table Section

      ImGui_bindings.set_function("TableSetColumnIndex", TableSetColumnIndext);

      /// Layout Section

      ImGui_bindings.set_function("BeginVertical", BeginVertical);
      ImGui_bindings.set_function("EndVertical", EndVertical);

      ImGui_bindings.set_function("Separator", Separator);
      ImGui_bindings.set_function(
        "SameLine",
        sol::overload(
          sol::resolve<void()>(SameLine),
          sol::resolve<void(float)>(SameLine)
        )
      );
      ImGui_bindings.set_function("NewLine", NewLine);
      ImGui_bindings.set_function("Spacing", Spacing);
      ImGui_bindings.set_function("Dummy", Dummy);

      ImGui_bindings.set_function(
        "Indent",
        sol::overload(
          sol::resolve<void()>(Indent),
          sol::resolve<void(float)>(Indent)
        )
      );

      ImGui_bindings.set_function(
        "Unindent",
        sol::overload(
          sol::resolve<void()>(Unindent),
          sol::resolve<void(float)>(Unindent)
        )
      );

      ImGui_bindings.set_function("BeginGroup", BeginGroup);
      ImGui_bindings.set_function("EndGroup", EndGroup);
      ImGui_bindings.set_function("GetCursorPos", GetCursorPos);
      ImGui_bindings.set_function("GetCursorPosX", GetCursorPosX);
      ImGui_bindings.set_function("GetCursorPosY", GetCursorPosY);
      ImGui_bindings.set_function("SetCursorPos", SetCursorPos);
      ImGui_bindings.set_function("SetCursorPosX", SetCursorPosX);
      ImGui_bindings.set_function("SetCursorPosY", SetCursorPosY);
      ImGui_bindings.set_function("GetCursorStartPos", GetCursorStartPos);
      ImGui_bindings.set_function("GetCursorScreenPos", GetCursorScreenPos);
      ImGui_bindings.set_function("SetCursorScreenPos", SetCursorScreenPos);
      ImGui_bindings.set_function("AlignTextToFramePadding", AlignTextToFramePadding);
      ImGui_bindings.set_function("GetTextLineHeight", GetTextLineHeight);
      ImGui_bindings.set_function("GetTextLineHeightWithSpacing", GetTextLineHeightWithSpacing);
      ImGui_bindings.set_function("GetFrameHeight", GetFrameHeight);
      ImGui_bindings.set_function("GetFrameHeightWithSpacing", GetFrameHeightWithSpacing);

      /// Item Interaction Section

      ImGui_bindings.set_function("TempInputIsActive", TempInputIsActive);
      ImGui_bindings.set_function("IsItemDeactivatedAfterEdit", IsItemDeactivatedAfterEdit);

      /// Engine UI Widgets

      auto drag_float = sol::overload(
        [](const std::string& str, float* val) -> bool {
          return ui::DragFloat(str.c_str(), val);
        }
        // ,
        // [](const std::string& str, float* val, float speed) -> bool {
        //   return ui::DragFloat(str.c_str(), val, speed);
        // },
        // [](const std::string& str, float* val, float speed, float min, float max) -> bool {
        //   return ui::DragFloat(str.c_str(), val, speed, min, max);
        // },
        // [](const std::string& str, float* val, float speed, float min, float max, const char* format) -> bool {
        //   return ui::DragFloat(str.c_str(), val, speed, min, max, format);
        // },
        // [](const std::string& str, float* val, float speed, float min, float max, const char* format, int32_t flags) -> bool {
        //   return ui::DragFloat(str.c_str(), val, speed, min, max, format, static_cast<ImGuiSliderFlags>(flags));
        // }
      );

      lua_state.create_named_table(
        "OtherUI",
        "PushId", PushRandomID,
        "PopId", PopRandomID,

        "DrawVec2", DrawVec2Control,
        "EditVec2", EditVec2,

        "DrawVec3", DrawVec3Control,
        "EditVec3", EditVec3,

        "DrawVec4", DrawVec4Control,
        "EditVec4", EditVec4

        // "DragInt8", DragInt8,
        // "DragUInt8", DragUInt8,
        // "DragInt16", DragInt16,
        // "DragUInt16", DragUInt16,
        // "DragInt32", DragInt32,
        // "DragUInt32", DragUInt32,
        // "DragInt64", DragInt64,
        // "DragUInt64", DragUInt64,

        // "DragFloat", [](const std::string& str, float& val) -> bool {
        //   return ui::DragFloat(str.c_str(), val);
        // }
        // "DragDouble", DragDouble
      );
    }

  }  // namespace lua_script_bindings
}  // namespace other
