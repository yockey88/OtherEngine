/**
 * \file scripting/cs/cs_garbage_collector.hpp
 **/
#ifndef OTHER_ENGINE_CS_GARBAGE_COLLECTOR_HPP
#define OTHER_ENGINE_CS_GARBAGE_COLLECTOR_HPP

#include <map>

#include <mono/metadata/object.h>

namespace other {

  typedef uint32_t GcHandle;

  class CsGarbageCollector {
    public:
      static void Shutdown();

      static void Collect(bool blocking = false);

      static GcHandle NewHandle(MonoObject* object);
      static MonoObject* GetHandleObject(GcHandle handle);

      static bool IsValidHandle(GcHandle handle);
      static void FreeHandle(GcHandle handle);

    private:
      using RefMap = std::map<GcHandle , MonoObject*>;
      static RefMap active_refs;
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_GARBAGE_COLLECTOR_HPP
