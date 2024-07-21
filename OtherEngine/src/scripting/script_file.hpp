/**
 * \file scripting/script_file.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_FILE_HPP
#define OTHER_ENGINE_SCRIPT_FILE_HPP

#include "asset/asset.hpp"

namespace other {

  class ScriptFile : public Asset {
    public:
      OE_ASSET(SCRIPTFILE);
    
      ScriptFile(const Path& path)
        : path(path) {}

    private:
      Path path;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_FILE_HPP
