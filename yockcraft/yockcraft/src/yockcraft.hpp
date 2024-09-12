/**
 * \file yockcraft.hpp
 **/
#ifndef YOCKCRAFT_HPP
#define YOCKCRAFT_HPP

#include "other_engine.hpp"

using namespace other;

class YockCraft : public App {
  public:
    YockCraft(const CmdLine& cmd_line, const ConfigTable& config) 
      : App(cmd_line, config) {}
    virtual ~YockCraft() override {}

    virtual void OnAttach() override;
    virtual void OnSceneLoad(const SceneMetadata* path) override;
};

OE_APPLICATION(YockCraft);

#endif // !YOCKCRAFT_HPP
