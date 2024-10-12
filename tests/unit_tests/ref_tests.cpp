/**
 * \file unit_tests/ref_tests.cpp
 **/
#include "oetest.hpp"

#include "core/ref.hpp"

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

TEST_F(RefTests , basic_tests) {
  Ref<TestObj> test_obj = other::NewRef<TestObj>(); 
  {
    Ref<TestObj> copy = test_obj;

    ASSERT_EQ(test_obj->Count() , 2);
    ASSERT_EQ(copy->Count() , 2);
    EXPECT_EQ(test_obj->data , copy->data);
    
    Ref<TestObj> copy2 = Ref<TestObj>::Clone(copy);

    ASSERT_EQ(copy->Count() , 3);
    ASSERT_EQ(copy2->Count() , 3);
    EXPECT_EQ(copy->data , copy2->data);
  }

  ASSERT_EQ(test_obj->Count() , 1);
  test_obj = nullptr;

  ASSERT_EQ(test_obj , nullptr);
}