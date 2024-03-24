/**
 * \file rendering/ui/file_explorer.cpp
*/
#include "rendering/ui/file_explorer.hpp"

#include <stdexcept>

namespace other {

  FileExplorer::FileExplorer(
    const std::string& title , const std::string& path , 
    FileExplorerType type
  ) : UIWindow(title) , 
      function(
        type , 
        [this]() { result = NFD_OpenDialog(nullptr , nullptr , &out_path); } ,
        [this]() { result = NFD_PickFolder(nullptr , &out_path); } ,
        [this]() { throw std::runtime_error("UNIMPLEMENTED"); } // result = NFD_SaveDialog(nullptr , nullptr , &out_path); }
      ) ,  path(path) {}

  void FileExplorer::OnAttach() {
    function();
    if (result == NFD_OKAY) {
      path = std::string{ out_path };
    }
  }

} // namespace other
