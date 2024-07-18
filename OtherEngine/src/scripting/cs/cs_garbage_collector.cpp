/**
 * \file scripting/cs/cs_garbage_collector.cpp
 **/
#include "scripting/cs/cs_garbage_collector.hpp"


#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>
#include <mono/metadata/profiler.h>

#include "core/logger.hpp"

namespace other {

  CsGarbageCollector::RefMap CsGarbageCollector::active_refs = {};

  void CsGarbageCollector::Shutdown() {
    for (auto& [handle , obj] : active_refs) {
      mono_gchandle_free(handle);
    }
    active_refs.clear();

    mono_gc_collect(mono_gc_max_generation());
    while (mono_gc_pending_finalizers() > 0) {}
  }

  void CsGarbageCollector::Collect(bool blocking) {
    mono_gc_collect(mono_gc_max_generation());
    if (blocking) {
      while (mono_gc_pending_finalizers() > 0) {}
    }
  }

  GcHandle CsGarbageCollector::NewHandle(MonoObject* object) {
    GcHandle handle = mono_gchandle_new(object , false);

    OE_ASSERT(handle != 0 , "Failed to create C# Garbage Collector handle");
    OE_ASSERT(active_refs.find(handle) == active_refs.end() , "Attempting to recreate C# Garbage Collector handle");
    active_refs[handle] = object;

    return handle;
  }

  MonoObject* CsGarbageCollector::GetHandleObject(GcHandle handle) {
    MonoObject* mobj = mono_gchandle_get_target(handle);
    if (mobj == nullptr || mono_object_get_vtable(mobj) == nullptr) {
      return nullptr;
    }

    return mobj;
  }

  bool CsGarbageCollector::IsValidHandle(GcHandle handle) {
    if (handle == 0) {
      return false;
    }

    MonoObject* mobj = mono_gchandle_get_target(handle);
    if (mobj == nullptr) {
      return false;
    }

    if (mono_object_get_vtable(mobj) == nullptr) {
      return false;
    }

    return true;
  }

  void CsGarbageCollector::FreeHandle(GcHandle handle) {
    if (handle == 0) {
      OE_ERROR("Attempting to free null C# Garbage Collector handle");
      return;
    }

    auto ref = active_refs.find(handle);
    if (ref == active_refs.end()) {
      // OE_ERROR("Failed to release C# Garbage Collector handle");
      return;
    }

    active_refs.erase(handle);
  }

} // namespace other
