/**
 * \file core/rand.cpp
 **/
#include "core/rand.hpp"


namespace other {

  RandomGenerator<uint64_t> Random::gen64;

  uint64_t Random::Generate() {
    return gen64.Next();
  }

  UUID Random::GenerateUUID() {
    return UUID(gen64.Next());
  }

} // namespace other
