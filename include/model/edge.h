/*!
 *\file edge.h
 *\brief Edge connecting two ports on the flowchart graph.
 */
#ifndef EDGE_H
#define EDGE_H

#include <cstdint>
#include <string>

#include "model/node.h"

namespace Cgen
{
   using EdgeId = uint64_t;

   /*!
    *\brief Directed connection between two node ports.
    */
   struct Edge
   {
      EdgeId id = 0;
      NodeId fromNodeId = 0;
      std::string fromPort;
      NodeId toNodeId = 0;
      std::string toPort;
   };
} // namespace Cgen

#endif // EDGE_H
