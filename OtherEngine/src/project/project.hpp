/**
 * \file project/project.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_HPP
#define OTHER_ENGINE_PROJECT_HPP

#include "core/ref_counted.hpp"

namespace other {
  
  class Project : public RefCounted {
    public:
      Project() {}
      virtual ~Project() override;

      Project(Project&& other);
      Project(const Project& other);
      Project& operator=(Project&& other);
      Project& operator=(const Project& other);

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_HPP
