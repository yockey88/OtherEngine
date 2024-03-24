/**
 * \file rendering/ui/file_explorer.hpp
 **/
#ifndef OTHER_ENGINE_FILE_EXPLORER_HPP
#define OTHER_ENGINE_FILE_EXPLORER_HPP

#include <string>
#include <optional>

#include <nfd/nfd.h>

#include "rendering/ui/ui_window.hpp"

namespace other {

  /// TODO: redo all of this, it works for now but Im not sure I like it/will be useful

  enum class FileExplorerType {
    OPEN_FILE ,
    OPEN_FOLDER ,
    SAVE_FILE
  };

  struct FileExplorerFunction {
    FileExplorerType type;
    std::function<void()> open_file;
    std::function<void()> open_folder;
    std::function<void()> save_file;

    FileExplorerFunction(
      FileExplorerType type , 
      std::function<void()> open_file , 
      std::function<void()> open_folder ,
      std::function<void()> save_file
    ) : type(type) , open_file(open_file) , open_folder(open_folder) {}

    void operator()() {
      switch (type) {
        case FileExplorerType::OPEN_FILE: open_file(); break;
        case FileExplorerType::OPEN_FOLDER: open_folder(); break;
        case FileExplorerType::SAVE_FILE: save_file(); break;
        default: break;
      }
    }
  };
 
  class FileExplorer : public UIWindow {
    FileExplorerFunction function;
    std::string path;

    nfdchar_t* out_path = nullptr;
    nfdresult_t result;

    public:
      FileExplorer(
        const std::string& title , const std::string& path = "." , 
        FileExplorerType type = FileExplorerType::OPEN_FILE
      );
      virtual ~FileExplorer() {}

      virtual void OnAttach() override;
      std::string GetPath() const { return path; }
  };

} // namespace enginey

#endif // !OTHER_ENGINE_FILE_EXPLORER_HPP
