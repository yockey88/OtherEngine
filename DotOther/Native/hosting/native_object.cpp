/**
 * \file native_object.cpp
 **/
#include "hosting/native_object.hpp"

#include "hosting/interop_interface.hpp"
#include "reflection/object_proxy.hpp"

namespace dotother {

  NObject::NObject(uint64_t handle) {
    if (handle == 0) {
      return;
    }
    this->object_handle = handle;
    proxy = new echo::ObjectProxy<NObject>(this);
    InteropInterface::Instance().RegisterObject(handle, this);
  }

  NObject::~NObject() {
    if (object_handle != 0) {
      InteropInterface::Instance().UnregisterObject(object_handle);
    }
    object_handle = 0;

    delete proxy;
    proxy = nullptr;
  }

}  // namespace dotother