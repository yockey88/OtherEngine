/**
 * \file env_memory_tests.cpp
 **/
#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>

#include "environment/env_defines.hpp"
#include "environment/memory.hpp"

#include "oetest.hpp"

using namespace other;

class MemoryTests : public OtherTest {
 protected:
  void SetUp() override {
    srand(time(nullptr));
  }

  void TearDown() override {
    expected_data = {};
    addresses = {};
  }

  glm::vec4 GetRandomVector() {
    return {
      rand(),
      rand(),
      rand(),
      rand()
    };
  }

  std::array<glm::vec4, Memory::kStackSize> expected_data;
  std::array<address_t, Memory::kStackSize> addresses;

  std::array<glm::vec4, Memory::kHeapSize> heap_data;
  std::array<address_t, Memory::kHeapSize> heap_addresses;
};

TEST_F(MemoryTests, simple_memory_test) {
  Memory memory;
  address_t addr;
  ASSERT_NO_THROW(addr = memory.Alloc<int32_t>(42));
  ASSERT_EQ(addr.page, 0);
  ASSERT_EQ(addr.page_idx, 0);

  int32_t ptr = memory.Get<int32_t>(addr);
  ASSERT_EQ(ptr, 42);

  glm::vec4 value = { 1.0f, 2.0f, 3.0f, 4.0f };
  ASSERT_NO_THROW(addr = memory.Push(value));
  ASSERT_EQ(addr.page, Memory::kStackPage);
  ASSERT_EQ(addr.page_idx, 0);

  glm::vec4& vec4 = memory.Get<glm::vec4>(addr);
  ASSERT_EQ(vec4, value);

  vec4.x = 5.0f;

  glm::vec4 popped = memory.Pop<glm::vec4>();
  glm::vec4 expected = { 5.0f, 2.0f, 3.0f, 4.0f };
  ASSERT_EQ(popped, expected);
}

TEST_F(MemoryTests, stack_testing) {
  Memory memory;
  for (uint32_t i = 0; i < Memory::kStackSize; ++i) {
    expected_data[i] = GetRandomVector();
    ASSERT_NO_THROW(addresses[i] = memory.Push(expected_data[i]));
  }

  ASSERT_EQ(memory.StackCursorPos(), Memory::kStackEnd);
  ASSERT_THROW(memory.Push(GetRandomVector()), std::out_of_range);

  for (uint32_t i = 0; i < Memory::kStackSize; ++i) {
    ASSERT_EQ(memory.Get<glm::vec4>(addresses[i]), expected_data[i]);
  }

  for (uint32_t i = 0; i < Memory::kStackSize; ++i) {
    glm::vec4 popped = memory.Pop<glm::vec4>();
    ASSERT_EQ(popped, expected_data[Memory::kStackSize - i - 1]);
  }
}

TEST_F(MemoryTests, heap_testing) {
  Memory memory;
  for (uint32_t i = 0; i < Memory::kHeapSize; ++i) {
    heap_data[i] = GetRandomVector();
    ASSERT_NO_THROW(heap_addresses[i] = memory.Alloc(heap_data[i]));
  }

  ASSERT_THROW(memory.Alloc(GetRandomVector()), std::out_of_range);

  for (uint32_t i = 0; i < Memory::kHeapSize; ++i) {
    ASSERT_EQ(memory.Get<glm::vec4>(heap_addresses[i]), heap_data[i]);
  }

  for (uint32_t i = 0; i < Memory::kHeapSize; ++i) {
    ASSERT_NO_THROW(memory.Free(heap_addresses[i]));
  }
}
