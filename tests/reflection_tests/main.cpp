#include <iostream>

#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

struct TT {};

struct TestType : TT {
  int a = 10;

  int GetB() {
    return b;
  }

  int b = 20;
};

void DestroyTestType(TestType& tt) {
  std::cout << "destroying test type\n"; 
}

static std::string gstring = "global string";

int main() {
  using namespace entt::literals;

  auto factory = entt::meta<TestType>().type("TESTTYPE"_hs);
  factory.ctor<>();
  factory.dtor<DestroyTestType>();

  factory.data<&TestType::a>("A"_hs);
  factory.data<&gstring>("attached_string"_hs);

  factory.data<nullptr , &TestType::b>("B"_hs);

  factory.base<TT>();

  auto my_t = entt::resolve("TESTTYPE"_hs);
  entt::meta_any a = my_t.data("A"_hs).get({});

  a.assign(11);

  return 0;
}
