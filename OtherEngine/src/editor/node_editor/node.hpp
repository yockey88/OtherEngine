/**
 * \file editor/node_editor/node.hpp
 **/
#ifndef OTHER_ENGINE_NODE_HPP
#define OTHER_ENGINE_NODE_HPP

#include <functional>

#include <imgui_node_editor/imgui_node_editor.h>

#include "core/uuid.hpp"
#include "core/value.hpp"

namespace other {

#if 0
  enum class EdgeEndpointType {
    OUTPUT , 
    INPUT 
  };

  struct Node;

  struct EdgeEndpoint {
    EdgeEndpointType type; 
  };

  struct Node {
    UUID id;
    std::string name;
  };

  struct Edge {
    UUID id;

    UUID input;
    UUID output;

    Edge(UUID in , UUID out)
      : input(in) , output(out) {}
  };
#endif
  
  using EdgeInput = std::function<void(const ValueList&)>;
  using EdgeOutput = std::function<ValueList()>;

  struct Edge {
    EdgeInput input;
    EdgeOutput output;
  };

  using OutputGroup = std::map<UUID , Edge>;
  using InputGroup = std::vector<Edge>;

  struct Node {
    OutputGroup outputs;

    virtual void AcceptInput(const std::vector<ValueList>& values) = 0;
    virtual std::vector<ValueList> RetrieveOutput() = 0;
  };

} // namespace other

#endif // !OTHER_ENGINE_NODE_HPP
