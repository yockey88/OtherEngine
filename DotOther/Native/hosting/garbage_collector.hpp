/**
 * \file hosting/garbage_collector.hpp
 **/
#ifndef DOTOTHER_GARBAGE_COLLECTOR_HPP
#define DOTOTHER_GARBAGE_COLLECTOR_HPP

#include <cstdint>

namespace dotother {

  enum class GCMode {
    DEFAULT = 0,
    FORCED = 1,
    OPTIMIZED = 2,
    AGGRESSIVE = 3
  };

  class GarbageCollector {
    public:
      static void Collect();
      static void CompactCollect();
      static void Collect(int32_t context_id , GCMode mode , bool blocking , bool compacting);
      static void WaitForPendingFinalizers(int32_t context_id);
  };

} // namespace dotother

#endif // !DOTOTHER_GARBAGE_COLLECTOR_HPP