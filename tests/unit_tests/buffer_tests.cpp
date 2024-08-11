/**
 * \file tests/unit_tests/buffer_tests.cpp
 **/
#include "oetest.hpp"

#include "core/buffer.hpp"

#include "unit_tests/oetest.hpp"

using other::Buffer;

class BufferTests : public other::OtherTest {
  public:
    Buffer buffer;

    void SetUp() override {
      other::OtherTest::SetUp();
      ASSERT_EQ(buffer.Size() , 0u) <<
        "Buffer size is not initially zero!";
    }

    void TearDown() override {
      ASSERT_NO_FATAL_FAILURE(std::cout << "END OF TEST DUMP :\n" << buffer.DumpBuffer() << "\n");
      ASSERT_NO_FATAL_FAILURE(buffer.Release());
      other::OtherTest::TearDown();
    }
};

TEST_F(BufferTests , safe_double_release) {
  int32_t val = 333;
  Buffer b;
  EXPECT_NO_FATAL_FAILURE(b.Write(val));
  EXPECT_NO_FATAL_FAILURE(b.Release());
  EXPECT_NO_FATAL_FAILURE(b.Release());
  /// will attempt release in TearDown and we should be fine
}

TEST_F(BufferTests , simple_test) {
  // GTEST_SKIP();
  int32_t value = 33u;

  EXPECT_NO_FATAL_FAILURE(buffer.Write(value));
  EXPECT_EQ(buffer.Size() , sizeof(int32_t));  
  EXPECT_EQ(buffer.Read<int32_t>() , value);
  EXPECT_EQ(buffer.NumElements() , 1);
  EXPECT_EQ(buffer.At<int32_t>(0) , value);
}

TEST_F(BufferTests , array_test) {
  // GTEST_SKIP();
  std::array<size_t , 10> arr = {
    1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10
  };

  EXPECT_NO_FATAL_FAILURE(buffer.WriteArr<size_t>(arr));
  EXPECT_EQ(buffer.Size() , sizeof(size_t) * arr.size());
  ASSERT_EQ(buffer.NumElements() , 10);

  for (uint32_t i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.At<size_t>(i) , arr[i]) <<
      "Failed on .At<> test on step " << i;
  }
}

/// tests writing glm types to copy into uniform on draw
TEST_F(BufferTests , uniform_buffer_compatability) {
  using namespace std::string_view_literals;

  static_assert(sizeof(glm::mat4) == 4 * 4 * sizeof(float) , "mat4 type has unexpected size!");
  static_assert(sizeof(glm::vec4) == 4 * sizeof(float) , "vec4 type has unexpeccted size!");

  {
    glm::mat4 transform = glm::mat4(1.f);
    glm::vec4 vector = glm::vec4(1.f);

    /// after first write:
    ///   capacity == sizeof(glm::mat4) * 64 == 64 * 64 == 4096
    ///   size == 64
    /// then we shouldn't have to resize again in this test there should be no more resizes
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(transform));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , sizeof(glm::mat4));
    EXPECT_EQ(buffer.NumElements() , 1);
    
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(vector));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , sizeof(glm::mat4) + sizeof(glm::vec4));
    EXPECT_EQ(buffer.NumElements() , 2);
    
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(transform));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , 2 * sizeof(glm::mat4) + sizeof(glm::vec4));
    EXPECT_EQ(buffer.NumElements() , 3);
    
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(vector));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , 2 * sizeof(glm::mat4) + 2 * sizeof(glm::vec4));
    EXPECT_EQ(buffer.NumElements() , 4);
    
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(transform));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , 3 * sizeof(glm::mat4) + 2 * sizeof(glm::vec4));
    EXPECT_EQ(buffer.NumElements() , 5);
    
    EXPECT_NO_FATAL_FAILURE(buffer.BufferData(vector));
    EXPECT_EQ(buffer.Capacity() , sizeof(glm::mat4) * 64);
    EXPECT_EQ(buffer.Size() , 3 * sizeof(glm::mat4) + 3 * sizeof(glm::vec4));
    EXPECT_EQ(buffer.NumElements() , 6);
  }
    
  /// these are different items, so we know that the data written is not dependent on memory owned by 
  ///   another object (for example coppying over a pointer into the buffer rather than the data pointed to)
  glm::mat4 transform = glm::mat4(1.f);
  glm::vec4 vector = glm::vec4(1.f);
  
  ASSERT_EQ(buffer.ElementSize(0) , sizeof(glm::mat4));
  const glm::mat4& m0 = buffer.At<glm::mat4>(0);
  EXPECT_EQ(m0 , transform);

  ASSERT_EQ(buffer.ElementSize(1) , sizeof(glm::vec4));
  const glm::vec4& v0 = buffer.At<glm::vec4>(1);
  EXPECT_EQ(v0 , vector);
  
  ASSERT_EQ(buffer.ElementSize(2) , sizeof(glm::mat4));
  const glm::mat4& m1 = buffer.At<glm::mat4>(2);
  EXPECT_EQ(m1 , transform);
  
  ASSERT_EQ(buffer.ElementSize(3) , sizeof(glm::vec4));
  const glm::vec4& v1 = buffer.At<glm::vec4>(3);
  EXPECT_EQ(v1 , vector);
  
  ASSERT_EQ(buffer.ElementSize(4) , sizeof(glm::mat4));
  const glm::mat4& m2 = buffer.At<glm::mat4>(4);
  EXPECT_EQ(m2 , transform);
  
  ASSERT_EQ(buffer.ElementSize(5) , sizeof(glm::vec4));
  const glm::vec4& v2 = buffer.At<glm::vec4>(5);
  EXPECT_EQ(v2 , vector);

  ASSERT_DEATH(buffer.At<glm::vec4>(0) , "");
  ASSERT_DEATH(buffer.At<glm::mat4>(1) , "");
  ASSERT_DEATH(buffer.At<glm::vec4>(2) , "");
  ASSERT_DEATH(buffer.At<glm::mat4>(3) , "");
  ASSERT_DEATH(buffer.At<glm::vec4>(4) , "");
  ASSERT_DEATH(buffer.At<glm::mat4>(5) , "");
}
