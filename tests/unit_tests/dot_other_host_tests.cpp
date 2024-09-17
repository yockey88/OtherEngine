/**
 * \file hostfxr_tests.cpp
 **/
#include "host.hpp"

#include "unit_tests/oetest.hpp"
#include <gtest.h>

using namespace dotother;
using namespace other;

class HostTests : public other::OtherTest {
  protected:
};

TEST_F(HostTests, LoadHost) {
  // Host host;
  // HostConfig config = {
  //   .host_config_path = "./DotOther/Managed/DotOther.Managed.runtimeconfig.json",
  // };

  // ASSERT_TRUE(host.LoadHost(config));

  // ASSERT_NO_FATAL_FAILURE(host.UnloadHost());
}
