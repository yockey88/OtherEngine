/**
 * \file scripting/cs/cs_logging_bindings.cpp
 **/
#include "scripting/cs/cs_logging_bindings.hpp"

#include "core/defines.hpp"

namespace other {
namespace cs_script_bindings {


  void Write(const dotother::NString string , Logger::Level level) {
    std::string msg = string;
    switch (level) {
      case Logger::Level::TRACE:
        OE_TRACE("[ C# Trace ]> {}" , string);
        break;
      case Logger::Level::DEBUG:
        OE_DEBUG("[ C# Debug ]> {}" , string);
        break;
      case Logger::Level::INFO:
        OE_INFO("[ C# Info ]> {}" , string);
        break;
      case Logger::Level::WARN:
        OE_WARN("[ C# Warning ]> {}" , string);
        break;
      case Logger::Level::ERR:
        OE_ERROR("[ C# Error ]> {}" , string);
        break;
      case Logger::Level::CRITICAL:
        OE_CRITICAL("[ C# Critical ]> {}" , string);
        break;
      default:
        println("[ C# Unknown ]> {}" , string);
        break;
    }
  }

} // namespace cs_script_bindings
} // namespace other
