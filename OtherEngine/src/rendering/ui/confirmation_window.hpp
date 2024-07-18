/**
 * \file rendering/ui/confirmation_window.hpp
 **/
#ifndef OTHER_ENGINE_CONFIRMATION_WINDOW_HPP
#define OTHER_ENGINE_CONFIRMATION_WINDOW_HPP

#include "rendering/ui/ui_window.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  class ConfirmationWindow : public UIWindow {
    public: 
      ConfirmationWindow(const std::string& title , const std::string& warning_text , std::function<void()> action)
          : UIWindow(title) , warning_text(warning_text) , action(action) {
        PushRenderFunction(UI_FUNC(ConfirmationWindow::AskForConfirmation));
      }
      
    private:
      std::string warning_text = "";
      std::function<void()> action = nullptr;

      void AskForConfirmation() {
        {
          ScopedColor red(ImGuiCol_Text , ui::theme::red);
          ImGui::Text("%s" , warning_text.c_str());
        }
        ui::Underline();

        if (ImGui::Button("Confirm")) {
          action();
          Close();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
          Close();
        }
      }
  }; 

} // namespace other

#endif // !OTHER_ENGINE_CONFIRMATION_WINDOW_HPP
