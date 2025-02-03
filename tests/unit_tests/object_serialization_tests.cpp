/**
 * \file byte_writer_tests.cpp
 **/
#include <cstdint>
#include <queue>
#include <type_traits>

#include <glm/gtc/quaternion.hpp>
#include <refl/refl.hpp>

#include "core/buffer.hpp"
#include "engine/engine.hpp"
#include "memory/buffer_allocator.hpp"

#include "ecs/components/transform.hpp"

#include "oetest.hpp"
#include "serialization/object_serializer.hpp"

using namespace other;

struct serializable : refl::attr::usage::field, refl::attr::usage::function {};

class ObjectSerializationTests : public OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  virtual void SetUp() override {}
  virtual void TearDown() override {}

  static inline Scope<Engine> driver = nullptr;
};

struct MyObj {
  int x = 0;
  float y = 0.0f;
  char z = 'a';
};

ECHO_TYPE(
  type(MyObj),
  field(x, echo::serializable_field()),
  field(y, echo::serializable_field()),
  field(z, echo::serializable_field())
);

struct MyOtherObj {
  std::string name;
};

ECHO_TYPE(
  type(MyOtherObj),
  field(name, echo::serializable_field())
);

struct MyObjSerializer : public ObjectSerializer<MyObj, 3> {
  MyObjSerializer() {
    AddField<int, 0>(&MyObj::x);
    AddField<float, 1>(&MyObj::y);
    AddField<char, 2>(&MyObj::z);
  }
};

struct MyOtherObjSerializer : public ObjectSerializer<MyOtherObj, 1> {
  MyOtherObjSerializer() {
    AddReaderWriterForField<std::string, 0>(
      [](ByteBuffer& buffer, const MyOtherObj& obj) {
        buffer.BufferData<std::string>(obj.name);
      },
      [](ByteBuffer& buffer, size_t& offset, MyOtherObj& obj) {
        obj.name = buffer.ReadStr(offset);
        offset += obj.name.size() + 1;
      }
    );
  }
};

TEST_F(ObjectSerializationTests, write_test_obj) {
  ByteBuffer buffer;
  MyObjSerializer serializer;

  MyObj obj;
  obj.x = 10;
  obj.y = 3.14f;
  obj.z = 'b';
  {
    std::stringstream ss;
    ss << "Writing object:\n";
    ss << "x = " << obj.x << "\n";
    ss << "y = " << obj.y << "\n";
    ss << "z = " << obj.z << "\n";
    OE_TRACE(ss.str());
  }
  serializer.Write(buffer, obj);

  std::stringstream ss;
  ss << buffer.DumpBuffer();
  OE_TRACE(ss.str());

  MyObj obj2 = serializer.Read(buffer, 0);
  {
    std::stringstream ss;
    ss << "Reading object:\n";
    ss << "x = " << obj2.x << "\n";
    ss << "y = " << obj2.y << "\n";
    ss << "z = " << obj2.z << "\n";
    OE_TRACE(ss.str());
  }

  ASSERT_EQ(obj.x, obj2.x);
  ASSERT_EQ(obj.y, obj2.y);
  ASSERT_EQ(obj.z, obj2.z);

  buffer.Release();

  Transform transform;
  transform.position = glm::vec3(1.0f, 2.0f, 3.0f);
  transform.qrotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  transform.erotation = glm::eulerAngles(transform.qrotation);
  transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

  TransformSnapshotter transform_serializer;
  transform_serializer.Write(buffer, transform);
  {
    std::stringstream ss;
    ss << buffer.DumpBuffer();
    OE_TRACE(ss.str());
  }

  Transform transform2 = transform_serializer.Read(buffer, 0);
  ASSERT_EQ(transform.position, transform2.position);
  ASSERT_EQ(transform.qrotation, transform2.qrotation);
  ASSERT_EQ(transform.erotation, transform2.erotation);
  ASSERT_EQ(transform.scale, transform2.scale);

  MyOtherObj other_obj;
  other_obj.name = "Test Name";

  buffer.Release();

  MyOtherObjSerializer other_serializer;
  other_serializer.Write(buffer, other_obj);
  {
    std::stringstream ss;
    ss << buffer.DumpBuffer();
    OE_TRACE(ss.str());
  }
  using namespace std::string_literals;
  ASSERT_EQ(buffer.Size(), ("Test Name"s).size() + 1);

  MyOtherObj other_obj2 = other_serializer.Read(buffer, 0);
  ASSERT_EQ(other_obj.name, other_obj2.name);
}

TEST_F(ObjectSerializationTests, refl_test) {
  ByteBuffer buffer;

  MyObj obj;
  obj.x = 10;
  obj.y = 3.14f;
  obj.z = 'b';
  Serialize(buffer, std::forward<MyObj>(obj));
  {
    std::stringstream ss;
    ss << buffer.DumpBuffer();
    OE_TRACE(ss.str());
  }

  buffer.Release();

  Transform transform;
  transform.position = glm::vec3(1.0f, 2.0f, 3.0f);
  transform.qrotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  transform.erotation = glm::eulerAngles(transform.qrotation);
  transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
  Serialize(buffer, std::forward<Transform>(transform));
  {
    std::stringstream ss;
    ss << buffer.DumpBuffer();
    OE_TRACE(ss.str());
  }

  TransformSnapshotter transform_serializer;
  Transform transform2 = transform_serializer.Read(buffer, 0);
  ASSERT_EQ(transform.position, transform2.position);
  ASSERT_EQ(transform.qrotation, transform2.qrotation);
  ASSERT_EQ(transform.erotation, transform2.erotation);
  ASSERT_EQ(transform.scale, transform2.scale);
  {
    std::stringstream ss;
    ss << "Transform 1:\n";
    ss << "position = " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << "\n";
    ss << "qrotation = " << transform.qrotation.x << ", " << transform.qrotation.y << ", " << transform.qrotation.z << ", " << transform.qrotation.w << "\n";
    ss << "erotation = " << transform.erotation.x << ", " << transform.erotation.y << ", " << transform.erotation.z << "\n";
    ss << "scale = " << transform.scale.x << ", " << transform.scale.y << ", " << transform.scale.z << "\n";
    OE_TRACE(ss.str());
  }
  {
    std::stringstream ss;
    ss << "Transform 2:\n";
    ss << "position = " << transform2.position.x << ", " << transform2.position.y << ", " << transform2.position.z << "\n";
    ss << "qrotation = " << transform2.qrotation.x << ", " << transform2.qrotation.y << ", " << transform2.qrotation.z << ", " << transform2.qrotation.w << "\n";
    ss << "erotation = " << transform2.erotation.x << ", " << transform2.erotation.y << ", " << transform2.erotation.z << "\n";
    ss << "scale = " << transform2.scale.x << ", " << transform2.scale.y << ", " << transform2.scale.z << "\n";
    OE_TRACE(ss.str());
  }
}

TEST_F(ObjectSerializationTests, scene_save_test) {
}

void ObjectSerializationTests::SetUpTestSuite() {
  Arena::Initialize();

  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level", "trace", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/object-serialization-tests.log", true);
  test_config.Add("project", "working-directory", "./tests", true);
  test_config.Add("project", "bin-dir", "C:/Yock/code/OtherEngine/bin/Debug");

  driver = NewScope<Engine>(test_config, cmdline, "Object Serialization Test Main Thread");

  AppState::Initialize(driver.get());
}

void ObjectSerializationTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  driver = nullptr;

  Arena::Shutdown();
}
