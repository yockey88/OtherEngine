/**
 * \file editor/node_editor/node.hpp
 **/
#ifndef OTHER_ENGINE_NODE_HPP
#define OTHER_ENGINE_NODE_HPP

#include "core/uuid.hpp"

namespace other {

  enum class EdgeEndpointType {
    OUTPUT , 
    INPUT 
  };

  enum class ValueType {
    VALUE ,
    ARRAY
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

} // namespace other

#endif // !OTHER_ENGINE_NODE_HPP
