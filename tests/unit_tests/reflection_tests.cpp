/**
 * \file unit_tests/reflection_tests.cpp
 **/
#include <gtest/gtest.h>
#include <refl/refl.hpp>

#include "core/type_data.hpp"

#include "application/app_state.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

#include "oetest.hpp"

using namespace other;

class ReflectionTests : public OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();
};

TEST_F(ReflectionTests, Test1) {
  Ref<Scene> scene = NewRef<Scene>();
  Entity* ent = scene->CreateEntity("TestEntity");
  OE_ASSERT(ent != nullptr, "Failed to create entity");
  EXPECT_EQ(ent->Name(), "TestEntity");

  // refl::type_descriptor desc = refl::reflect(*ent);
  // refl::type_descriptor desc2 = refl::reflect(*scene);
  // std::cout << desc.name << std::endl;

  // refl::util::for_each(desc.members, [&](auto field) {
  //   std::cout << field.name << std::endl;
  // });

  // std::cout << " --- " << std::endl;

  // std::cout << desc2.name << std::endl;
  // refl::util::for_each(desc2.members, [&](auto field) {
  //   std::cout << field.name << std::endl;
  // });

  std::cout << TypeData<Entity>().Name() << std::endl;

  // std::stringstream ss;
  // TypeData<Entity>().Serialize(ss, *ent);
  // std::cout << ss.str() << std::endl;
}

using namespace std::string_literals;
void ReflectionTests::SetUpTestSuite() {
  // ConfigTable test_config;
  // test_config.Add("log", "console-level", "debug", true);
  // test_config.Add("log", "file-level", "trace", true);
  // test_config.Add("log", "path", "logs/reflection-test.log", true);
  // Logger::Open(test_config);
  // Logger::Instance()->RegisterThread("Script Engine Test Main Thread");

  // AppState::Initialize(cmdline, test_config);
}

void ReflectionTests::TearDownTestSuite() {
  // ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  // ASSERT_NO_FATAL_FAILURE(CloseLog());
}