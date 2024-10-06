/**
 * \file hosting/garbage_collector.cpp
 **/
#include "hosting/garbage_collector.hpp"

#include "hosting/interop_interface.hpp"

namespace dotother {

  void GarbageCollector::Collect() {
    Collect(-1, GCMode::DEFAULT, true, false);
  }

  void GarbageCollector::CompactCollect() {
    Collect(-1, GCMode::DEFAULT, true, true);
  }

  void GarbageCollector::Collect(int32_t context_id, GCMode mode, bool blocking, bool compacting) {
    Interop().collect_garbage(context_id, mode, blocking, compacting);
  }

  void GarbageCollector::WaitForPendingFinalizers(int32_t context_id) {
    Interop().wait_for_pending_finalizers();
  }

} // namespace dotother