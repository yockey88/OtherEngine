/**
 * \file editor/editor.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_HPP
#define OTHER_ENGINE_EDITOR_HPP

#include <glad/glad.h>

#include "application/app.hpp"
#include "editor/editor_asset_handler.hpp"

namespace other {

  class Editor : public App {
    public:
      Editor(const CmdLine& cmd_line, const ConfigTable& config) 
          : App(cmd_line, config) {}
      virtual ~Editor() override {} 

    private:
      virtual Ref<AssetHandler> CreateAssetHandler() override {
        return NewRef<EditorAssetHandler>();
      }
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
