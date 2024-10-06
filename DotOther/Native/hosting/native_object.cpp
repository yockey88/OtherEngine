/**
 * \file native_object.cpp
 **/
#include "hosting/native_object.hpp"

#include "hosting/interop_interface.hpp"
#include "reflection/object_proxy.hpp"

namespace dotother {

  NObject::NObject(uint64_t handle) {
    this->handle = handle;
    proxy = new echo::ObjectProxy<NObject>(this);
    InteropInterface::Instance().RegisterObject(handle, this);
  }

  NObject::~NObject() {
    InteropInterface::Instance().UnregisterObject(handle);

    delete proxy;
    proxy = nullptr;
    handle = 0;
  }

} // namespace dotother