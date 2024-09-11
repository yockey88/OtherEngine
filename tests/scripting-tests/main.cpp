/*
 * \file tests/scripting-tests/main.cpp
 **/
#include <spdlog/fmt/fmt.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "core/defines.hpp"

using other::Path;

namespace py = pybind11;

class C {
  public:
    enum class Number : uint32_t {
      ZERO = 0 ,
      ONE = 1 , 
      TWO = 2 , 
      THREE = 3
    };

    static bool CheckNum(Number num) {
      return num == Number::THREE;
    }
};

PYBIND11_EMBEDDED_MODULE(engine , m) {
  py::class_<C>(m , "C")
    .def_static("CheckNum" , &C::CheckNum);

  pybind11::enum_<C::Number>(m , "Number")
    .value("ZERO" , C::Number::ZERO)
    .value("ONE" , C::Number::ONE)
    .value("TWO" , C::Number::TWO)
    .value("THREE" , C::Number::THREE);

  m.def("CheckNum" , &C::CheckNum);
}

PYBIND11_EMBEDDED_MODULE(mod , m) {
  m.doc() = "test module";
  m.def("add" , [](int i, int j) {
    return i + j;
  });
}

int main(int argc , char* argv[]) {
  py::scoped_interpreter guard{};

  auto mod = py::module_::import("mod");
  auto res = mod.attr("add")(1 , 2).cast<int32_t>();

  py::exec(R"(
    from engine import C
    from engine import Number
    print(C.CheckNum(Number.TWO))
  )");

  std::cout << res << std::endl;

  std::cout << "YAY!" << std::endl;
  return 0;
}
