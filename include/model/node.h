/*!
 *\file node.h
 *\brief Flowchart node definition and factory helpers.
 */
#ifndef NODE_H
#define NODE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "model/block_type.h"
#include "model/port.h"

namespace Cgen
{
   using NodeId = uint64_t;
   using PropertyMap = std::unordered_map<std::string, std::string>;

   /*!
    *\brief A single block instance on the canvas.
    */
   struct Node
   {
      NodeId id = 0;
      BlockType type = BlockType::Start;
      float posX = 0.0f;
      float posY = 0.0f;
      std::vector<Port> ports;
      PropertyMap properties;
   };

   /*!
    *\brief Creates a node of the given type with default ports and properties.
    *
    *\param[in] id Node identifier.
    *\param[in] blockType Block kind.
    *\param[in] posX Canvas X position.
    *\param[in] posY Canvas Y position.
    *\return Fully initialized node.
    */
   Node CreateNode(NodeId id, BlockType blockType, float posX, float posY);

   /*!
    *\brief Finds a port by name.
    *
    *\param[in] node Node to search.
    *\param[in] portName Port name.
    *\return Pointer to port or nullptr.
    */
   const Port* FindPort(const Node& node, std::string_view portName);

   /*!
    *\brief Finds a mutable port by name.
    *
    *\param[in,out] pNode Node to search.
    *\param[in] portName Port name.
    *\return Pointer to port or nullptr.
    */
   Port* FindPortMutable(Node* pNode, std::string_view portName);

   /*!
    *\brief Updates data-port CTypes from node type properties.
    *
    *\param[in,out] pNode Node to sync.
    */
   void SyncNodePortTypes(Node* pNode);

   /*!
    *\brief Shows or hides Printf/FilePrintf Arg ports from format and wires.
    *
    *\param[in,out] pNode Printf or FilePrintf node.
    *\param[in] pDocument Document used to detect wired Arg ports (nullable).
    */
   void SyncPrintfArgVisibility(Node* pNode, const class GraphDocument* pDocument);

   /*!
    *\brief Runs SyncNodePortTypes and Printf visibility for every node.
    *
    *\param[in,out] pDocument Document to sync.
    */
   void SyncAllNodePorts(class GraphDocument* pDocument);
} // namespace Cgen

#endif // NODE_H
