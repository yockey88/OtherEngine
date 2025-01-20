/**
 * \file profiling/profiling.hpp
 **/
#ifndef OTHER_ENGINE_PROFILING_HPP
#define OTHER_ENGINE_PROFILING_HPP

#ifdef OTHER_PROFILE_BUILD

  #include <glad/glad.h>

  #include "tracy/Tracy.hpp"
  #include "tracy/TracyOpenGL.hpp"

  #define ADD_MARK FrameMark
  #define ADD_NAMED_MARK(name) FrameMarkNamed(name)

  #define PROFILE_SCOPE ZoneScoped
  #define PROFILE_SECTION(name) ZoneScopedN(name)

  #define ADD_PROFILE_TAG(name) ZoneText(name, strlen(name))
  #define ADD_PROFILE_MESSAGE(message, size) TracyMessageS(message, size)
  #define PROFILE_PLOT_VALUE(name, value) TracyPlot(name, value)

  #define PROFILE_ALLOCATION(p, size) TracyAlloc(p, size)
  #define PROFILE_DEALLOCATION(p) TracyFree(p)

  #define PROFILE_GPU_CONTEXT TracyGpuContext

#else

  #define ADD_MARK ((void)0)
  #define ADD_NAMED_MARK(name) ((void)0)

  #define PROFILE_SCOPE ((void)0)
  #define PROFILE_SECTION(name) ((void)0)

  #define ADD_PROFILE_TAG(name) ((void)0)
  #define ADD_PROFILE_MESSAGE(message, size) ((void)0)
  #define PROFILE_PLOT_VALUE(name, value) ((void)0)

  #define PROFILE_ALLOCATION(p, size) ((void)0)
  #define PROFILE_DEALLOCATION(p) ((void)0)

  #define PROFILE_GPU_CONTEXT ((void)0)

#endif  // OTHER_PROFILE_BUILD

#endif  // !OTHER_ENGINE_PROFILING_HPP