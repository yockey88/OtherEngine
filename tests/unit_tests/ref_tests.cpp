/**
 * \file unit_tests/ref_tests.cpp
 **/
#include "core/ref.hpp"

#include "oetest.hpp"

using other::Ref;
using other::RefCounted;

class TestObj : public RefCounted {
 public:
  std::string data = "Test String";

  TestObj() {
    std::cout << "TestObj::TestObj()" << std::endl;
  }

  ~TestObj() {
    std::cout << "TestObj::~TestObj()" << std::endl;
  }
};

class RefTests : public other::OtherTest {
 public:
};

TEST_F(RefTests, basic_tests) {
  Ref<TestObj> test_obj = other::NewRef<TestObj>();
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
  {
    Ref<TestObj> copy = test_obj;

    ASSERT_EQ(test_obj->Count(), 2);
    ASSERT_EQ(copy->Count(), 2);
    EXPECT_EQ(test_obj->data, copy->data);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);

    Ref<TestObj> copy2 = Ref<TestObj>::Clone(copy);

    ASSERT_EQ(copy->Count(), 3);
    ASSERT_EQ(copy2->Count(), 3);
    EXPECT_EQ(copy->data, copy2->data);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
  }

  ASSERT_EQ(test_obj->Count(), 1);
  test_obj = nullptr;

  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 0);
  ASSERT_EQ(test_obj, nullptr);
}

TEST_F(RefTests, container_tests) {
  std::vector<Ref<TestObj>> objs;
  for (int i = 0; i < 10; i++) {
    objs.push_back(other::NewRef<TestObj>());
  }

  ASSERT_EQ(objs.size(), 10);
  for (auto& obj : objs) {
    ASSERT_EQ(obj->Count(), 1);
  }
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 10);

  {
    std::vector<Ref<TestObj>> objs2 = objs;
    ASSERT_EQ(objs2.size(), 10);
    for (auto& obj : objs2) {
      ASSERT_EQ(obj->Count(), 2);
    }
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 10);
  }

  for (auto& obj : objs) {
    ASSERT_EQ(obj->Count(), 1);
  }
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 10);

  objs.clear();
}

TEST_F(RefTests, pass_by_value) {
  auto func = [](Ref<TestObj> obj) {
    ASSERT_EQ(obj->Count(), 2);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
    obj = nullptr;
  };

  Ref<TestObj> obj = other::NewRef<TestObj>();
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);

  func(obj);
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);

  auto func2 = [](Ref<TestObj> obj) {
    ASSERT_EQ(obj->Count(), 2);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
  };

  func2(obj);
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
}

TEST_F(RefTests, pass_by_ref) {
  auto func = [](Ref<TestObj>& obj) {
    ASSERT_EQ(obj->Count(), 1);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
    obj = nullptr;
  };

  Ref<TestObj> obj = other::NewRef<TestObj>();
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);

  func(obj);
  ASSERT_EQ(obj, nullptr);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 0);

  obj = other::NewRef<TestObj>();
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);

  auto func2 = [](Ref<TestObj>& obj) {
    ASSERT_EQ(obj->Count(), 1);
    ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
  };

  func2(obj);
  ASSERT_EQ(obj->Count(), 1);
  ASSERT_EQ(other::detail::NumberOfLivingReferences(), 1);
}