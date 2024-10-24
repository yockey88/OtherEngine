/**
 * \file Native/unit_tests/reflection_test.cpp
 **/
#include <cstdint>
#include <iostream>

#include <core/dotother_defines.hpp>
#include <gtest.h>
#include <refl/refl.hpp>

#include "core/dotest.hpp"

#include "hosting/native_object.hpp"
#include "reflection/callable.hpp"
#include "reflection/echo.hpp"
#include "reflection/echo_defines.hpp"
#include "reflection/object_proxy.hpp"
#include "reflection/type_database.hpp"


struct MyStruct : echo::serializable {
  ECHO_REFLECT();

  float x;
  float y;
  float z;

  MyStruct()
      : x(0.f), y(0.f), z(0.f){};
  MyStruct(float x)
      : x(x), y(x), z(x) {}
  MyStruct(float x, float y, float z)
      : x(x), y(y), z(z) {}
};

using dotother::MessageLevel;

struct Foo : echo::callable, echo::static_callable {
  ECHO_REFLECT();

  uint32_t operator()() {
    dotother::util::print(DO_STR("Foo called"), MessageLevel::DEBUG);
    return 69;
  }

  static void Call(float f, uint32_t i) {
    dotother::util::print(DO_STR("Foo static called w args: {:f} , {}"), MessageLevel::DEBUG, f, i);
  }
};

struct Bar : echo::callable, echo::static_callable {
  ECHO_REFLECT();

  static float Call() {
    dotother::util::print(DO_STR("Bar static called"), MessageLevel::DEBUG);
    return 420.f;
  }

  void operator()(float f, uint32_t i) {
    dotother::util::print(DO_STR("Bar called w args: {:f} , {}"), MessageLevel::DEBUG, f, i);
  }
};

void debug_print(std::ostream& is, const MyStruct& strct) {
  is << "MyStruct { x: " << strct.x << ", y: " << strct.y << ", z: " << strct.z << " }";
}

ECHO_TYPE(
  type(MyStruct, debug(debug_print), bases<>),
  field(x, echo::serializable_field()),
  field(y, echo::serializable_field()),
  field(z, echo::serializable_field())
);

ECHO_TYPE(type(Foo, bases<>));

ECHO_TYPE(type(Bar, bases<>));

class ReflectionTests : public DoTest {
 public:
 protected:
  virtual void TearDown() override {
    dotother::echo::TypeDatabase::CloseDatabase();
  }

  template <typename T>
  void PrintTypeMembers_refl();
  template <typename T>
  void DumpDebugData_refl(const std::vector<T>& data);

  template <typename T>
  void SerializeData_refl(std::ostream& os, const std::vector<T>& data);
};

TEST_F(ReflectionTests, refl_test) {
  std::vector<MyStruct> structs{
    { 1.0f, 2.0f, 3.0f },
    { 4.0f, 5.0f, 6.0f },
    { 7.0f, 8.0f, 9.0f }
  };

  std::vector<Foo> foos{
    Foo{},
    Foo{},
    Foo{}
  };
  {
    std::stringstream ss;
    SerializeData_refl(ss, structs);
    EXPECT_NE(ss.str(), "");
  }

  {
    std::stringstream ss;
    /// should print nothing because Foo is not echo::serializable
    SerializeData_refl(ss, foos);
    EXPECT_EQ(ss.str(), "");
  }

  MyStruct test{ 2.f };
  echo::reflectable& r = test;
  const dotother::echo::TypeMetadata& tmd = r.ReadMetadata();
  EXPECT_EQ(tmd.name, "MyStruct");
  EXPECT_EQ(tmd.methods.size(), 0);
  EXPECT_EQ(tmd.fields.size(), 3);

  Foo foothing;
  uint32_t res = dotother::Invoke<Foo, uint32_t>(foothing);
  dotother::Invoke<Foo>(1.f, 2);

  EXPECT_EQ(res, 69);

  Bar barthing;
  float fres = dotother::Invoke<Bar, float>();
  EXPECT_EQ(fres, 420.f);
  dotother::Invoke<Bar>(barthing, 1.f, 2);

  EXPECT_EQ(fres, 420.f);
}

using namespace std::string_view_literals;
TEST_F(ReflectionTests, proxy_test) {
  dotother::NObject obj(0xdeadbeef);
  obj.proxy->object_handle(0xfeedbeef);

  EXPECT_EQ(obj.object_handle, 0xfeedbeef)
    << fmt::format("Expected object_handle to be {:#08x} but got {:#08x}"sv, 0xfeedbeef, obj.object_handle);
}

template <typename T>
void ReflectionTests::PrintTypeMembers_refl() {
  auto reflection = refl::reflect<T>();
  refl::util::for_each(reflection.members, [](auto member) {
    if constexpr (is_readable(member)) {
      std::cout << "Member: " << member.name << std::endl;
    }
  });
}

template <typename T>
void ReflectionTests::DumpDebugData_refl(const std::vector<T>& data) {
  refl::runtime::debug(std::cout, data);
  std::cout << std::endl;
}

template <typename T>
void ReflectionTests::SerializeData_refl(std::ostream& os, const std::vector<T>& data) {
  for (auto& item : data) {
    dotother::SerializeObject<T>(os, T{ item });
  }
}
