/**
 * \file project/project.cpp
 **/
#include "project/project.hpp"

namespace other {

  Project::~Project() {
    Decrement();
  }

  Project::Project(Project&& other) {
  }

  Project::Project(const Project& other) {
    Increment();
  }

  Project& Project::operator=(Project&& other) {
    return *this;
  }

  Project& Project::operator=(const Project& other) {
    Increment();
    return *this;
  }

} // namespace other
