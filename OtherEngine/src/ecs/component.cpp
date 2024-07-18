/**
 * \file ecs/component.cpp
 **/
#include "ecs/component.hpp"

#include <iterator>
#include <algorithm>

#include "core/defines.hpp"
#include "core/uuid.hpp"

namespace other {

  std::string ComponentDataBase::GetComponentTagUc(size_t idx) {
    auto itr = std::find_if(kComponentTags.begin() , kComponentTags.end() , [&idx](const auto& tag_pair) -> bool {
      return idx == tag_pair.second;
    });

    if (itr == kComponentTags.end()) {
      return "";
    }

    return std::string{ (*itr).first };
  }

  std::string ComponentDataBase::GetComponentTagLc(size_t idx) {
    auto itr = std::find_if(kComponentTags.begin() , kComponentTags.end() , [&idx](const auto& tag_pair) -> bool {
      return idx == tag_pair.second;
    });

    if (itr == kComponentTags.end()) {
      return "";
    }

    std::string lc_tag;
    std::transform((*itr).first.begin() , (*itr).first.end() , std::back_inserter(lc_tag) , ::tolower);

    return lc_tag; 
  }

  int32_t ComponentDataBase::GetComponentIdxFromTag(const std::string_view tag) {
    std::string uc_tag;
    std::transform(tag.begin() , tag.end() , std::back_inserter(uc_tag) , ::toupper);

    UUID hash = FNV(uc_tag);
    auto itr = std::find_if(kComponentTags.begin() , kComponentTags.end() , [&hash](const auto& tag_pair) -> bool {
      return hash.Get() == FNV(tag_pair.first);
    });

    if (itr == kComponentTags.end()) {
      return -1;
    }
    
    return (*itr).second;
  }

} // namespace other
