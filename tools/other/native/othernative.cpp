/**
 * \file native/othernative.cpp
 **/
#include <iostream>
#include <string_view>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "core/cmd_line.hpp"
#include "core/config_table.hpp"
#include "core/defines.hpp"
#include "core/formatters.hpp"
#include "core/logger.hpp"

#include "otherenv.hpp"

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<std::string>);

namespace other {

  class PyEnv {
   public:
    static Opt<Path> env_cfg_path;
    static CmdLine cmd_line;
    static Opt<ConfigTable> config;

    //// exit condition
    static Opt<ExitCode> exit_code;
  };

  struct LogWrapper {
    static void Trace(const std::string& msg);
    static void Debug(const std::string& msg);
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);
    static void Critical(const std::string& msg);
  };

}  // namespace other

PYBIND11_MODULE(othernative, m) {
  m.def("say_hello", []() { return "Hello, World!"; });
  m.def("say", [](const std::string& str) { other::println(str); });

  py::class_<std::vector<std::string>>(m, "VectorStr", py::module_local(true))
    .def(py::init<>())
    .def("push_back", [](std::vector<std::string>& v, const std::string& value) { v.push_back(value); });

  py::class_<other::LogWrapper>(m, "Logger")
    .def("trace", &other::LogWrapper::Trace)
    .def("debug", &other::LogWrapper::Debug)
    .def("info", &other::LogWrapper::Info)
    .def("warn", &other::LogWrapper::Warn)
    .def("error", &other::LogWrapper::Error)
    .def("critical", &other::LogWrapper::Critical);

  py::class_<other::CmdLine>(m, "CmdLine");

  py::enum_<other::ExitCode>(m, "ExitCode")
    .value("SUCCESS", other::ExitCode::SUCCESS)
    .value("FAILURE", other::ExitCode::FAILURE);

  m.def("OtherMain", [](const std::vector<std::string>& args) -> uint32_t {
    other::CmdLine cmd_line{ args };
    other::ExitCode code = other::Main(cmd_line);
    return code == other::ExitCode::SUCCESS ? 0 : 1;
  });
}

namespace other {

  Opt<Path> PyEnv::env_cfg_path = std::nullopt;

  CmdLine PyEnv::cmd_line;
  Opt<ConfigTable> PyEnv::config;

  Opt<ExitCode> PyEnv::exit_code = std::nullopt;

  void LogWrapper::Trace(const std::string& msg) {
    std::string msg_str = msg;
    OE_TRACE(" py > {}", msg_str);
  }
  void LogWrapper::Debug(const std::string& msg) {
    OE_DEBUG(" py > {}", msg);
  }
  void LogWrapper::Info(const std::string& msg) {
    OE_INFO(" py > {}", msg);
  }
  void LogWrapper::Warn(const std::string& msg) {
    OE_WARN(" py > {}", msg);
  }
  void LogWrapper::Error(const std::string& msg) {
    OE_ERROR(" py > {}", msg);
  }
  void LogWrapper::Critical(const std::string& msg) {
    OE_CRITICAL(" py > {}", msg);
  }

}  // namespace other
