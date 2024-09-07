/**
 * \file yockcraft.hpp
 **/
#ifndef YOCKCRAFT_HPP
#define YOCKCRAFT_HPP

#include "other_engine.hpp"

using namespace other;

class YockCraft : public App {
  public:
    YockCraft(other::Engine* engine) 
      : App(engine) {}
    virtual ~YockCraft() override {}

    virtual void OnAttach() override;
    virtual void OnSceneLoad(const SceneMetadata* path) override;
};

OE_APPLICATION(YockCraft);

#endif // !YOCKCRAFT_HPP
