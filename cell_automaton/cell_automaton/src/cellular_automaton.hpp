/**
 * \file cellular_automaton.hpp
 **/
#ifndef CELLULAR_AUTOMATON_HPP
#define CELLULAR_AUTOMATON_HPP

#include "core/config.hpp"

#include "application/app.hpp"
#include "parsing/cmd_line_parser.hpp"

using other::App;
using other::CmdLine;
using other::ConfigTable;

namespace cellauto {

  class CaApp : public App {
   public:
    CaApp(const CmdLine& cmdline, const ConfigTable& config) : App(cmdline, config) {}
    ~CaApp() {}

    // virtual void OnLoad() {}
    // virtual void OnAttach() {}

    // virtual void EarlyUpdate(float dt) {}
    // virtual void Update(float dt) {}
    // virtual void LateUpdate(float dt) {}
    // virtual void Render() {}
    // virtual void RenderUI() {}

    // virtual void OnDetach() {}
    // virtual void OnUnload() {}

    // virtual void OnSceneLoad(const SceneMetadata* path) {}
    // virtual void OnSceneUnload() {}
  };

}  // namespace cellauto

#endif  // !CELLULAR_AUTOMATON_HPP