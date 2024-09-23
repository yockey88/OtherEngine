/**
 * \file core/rand.hpp
 **/
#ifndef OTHER_ENGINE_CORE_RAND_HPP
#define OTHER_ENGINE_CORE_RAND_HPP

#include <random>

#include "core/uuid.hpp"

namespace other {

  template <typename T>
  concept numeric = std::is_integral_v<T>;
  
  template <numeric T>
  class RandomGenerator {
    public:
      RandomGenerator()
        : gen(rand()) {}
      RandomGenerator(T min , T max) 
        : gen(rand()) , dist(min , max) {}
      ~RandomGenerator() {}

      T Next() { return dist(gen); }

    private:
      std::random_device rand;
      std::mt19937 gen;
      std::uniform_int_distribution<T> dist;
  };

  class Random {
    public:
      static uint64_t Generate();
      static UUID GenerateUUID();

    private:
      static RandomGenerator<uint64_t> gen64;
  };

} // namespace other

#endif // !OTHER_ENGINE_CORE_RAND_HPP
