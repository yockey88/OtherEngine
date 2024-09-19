/**
 * \file garbage_collector.hpp
 **/
#ifndef DOTOTHER_GARBAGE_COLLECTOR_HPP
#define DOTOTHER_GARBAGE_COLLECTOR_HPP

namespace dotother {

  enum class GCMode {
    DEFAULT = 0,
    FORCED = 1,
    OPTIMIZED = 2,
    AGGRESSIVE = 3
  };

} // namespace dotother

#endif // !DOTOTHER_GARBAGE_COLLECTOR_HPP