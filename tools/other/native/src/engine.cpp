#include <pybind11/pybind11.h>

#include "core/defines.hpp"

using namespace other;
namespace py = pybind11;

class PythonInterface {
  public:
};

PYBIND11_MODULE(engine, m) {
  m.def("fnv_hash" , &FNV, "FNV hash function");
}