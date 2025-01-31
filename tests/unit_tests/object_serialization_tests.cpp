/**
 * \file byte_writer_tests.cpp
 **/
#include <cstdint>
#include <queue>

#include "core/buffer.hpp"
#include "memory/buffer_allocator.hpp"

#include "oetest.hpp"
#include "serialization/object_serializer.hpp"

using namespace other;

class ObjectSerializationTests : public OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

struct MyObj {
  int x = 0;
  float y = 0.0f;
  char z = 'a';
};

struct MyObjSerializer : public ObjectSerializer<MyObj, 3> {
  MyObjSerializer() {
    AddField<int, 0>(&MyObj::x);
    AddField<float, 1>(&MyObj::y);
    AddField<char, 2>(&MyObj::z);
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
}

void ObjectSerializationTests::SetUpTestSuite() {
  Arena::Initialize();

  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level", "trace", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/byte-writer-tests.log", true);

  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Byte Writer Test Main Thread");
}

void ObjectSerializationTests::TearDownTestSuite() {
  CloseLog();

  Arena::Shutdown();
}
