/*!
 *\file graph_clipboard.cpp
 *\brief Copy/paste helpers for flowchart subgraphs.
 */
#include "model/graph_clipboard.h"

#include <unordered_set>

#include "model/edge.h"
#include "model/node.h"

namespace Cgen
{
   void CopySelectionToClipboard(const GraphDocument& document,
                                 const std::vector<NodeId>& selectedNodeIds,
                                 GraphClipboard* pClipboard)
   {
      if (pClipboard == nullptr)
      {
         return;
      }
      pClipboard->nodes.clear();
      pClipboard->edges.clear();
      pClipboard->hasContent = false;

      std::unordered_set<NodeId> selected;
      std::vector<NodeId> orderedSelected;
      for (const NodeId nodeId : selectedNodeIds)
      {
         const Node* pNode = document.FindNode(nodeId);
         if (pNode == nullptr)
         {
            continue;
         }
         if (pNode->type == BlockType::Start)
         {
            continue;
         }
         if (selected.find(nodeId) != selected.end())
         {
            continue;
         }
         selected.insert(nodeId);
         orderedSelected.push_back(nodeId);
      }
      if (orderedSelected.empty())
      {
         return;
      }

      std::unordered_map<NodeId, NodeId> idToLocal;
      NodeId nextLocal = 1;
      for (const NodeId nodeId : orderedSelected)
      {
         const Node* pNode = document.FindNode(nodeId);
         if (pNode == nullptr)
         {
            continue;
         }
         ClipboardNode item;
         item.type = pNode->type;
         item.posX = pNode->posX;
         item.posY = pNode->posY;
         item.properties = pNode->properties;
         item.localId = nextLocal;
         idToLocal[nodeId] = nextLocal;
         ++nextLocal;
         pClipboard->nodes.push_back(item);
      }

      for (const Edge& edge : document.GetEdges())
      {
         if (selected.find(edge.fromNodeId) == selected.end())
         {
            continue;
         }
         if (selected.find(edge.toNodeId) == selected.end())
         {
            continue;
         }
         ClipboardEdge item;
         item.fromLocalId = idToLocal[edge.fromNodeId];
         item.fromPort = edge.fromPort;
         item.toLocalId = idToLocal[edge.toNodeId];
         item.toPort = edge.toPort;
         pClipboard->edges.push_back(item);
      }

      pClipboard->hasContent = !pClipboard->nodes.empty();
   }

   bool PasteClipboardIntoDocument(GraphDocument* pDocument,
                                   const GraphClipboard& clipboard,
                                   float offsetX,
                                   float offsetY,
                                   std::vector<NodeId>* pOutPastedIds)
   {
      if ((pDocument == nullptr) || (!clipboard.hasContent))
      {
         return false;
      }
      if (pOutPastedIds != nullptr)
      {
         pOutPastedIds->clear();
      }

      std::unordered_map<NodeId, NodeId> localToNew;
      for (const ClipboardNode& item : clipboard.nodes)
      {
         const NodeId newId =
            pDocument->AddNode(item.type, item.posX + offsetX, item.posY + offsetY);
         Node* pNode = pDocument->FindNodeMutable(newId);
         if (pNode != nullptr)
         {
            pNode->properties = item.properties;
         }
         localToNew[item.localId] = newId;
         if (pOutPastedIds != nullptr)
         {
            pOutPastedIds->push_back(newId);
         }
      }

      for (const ClipboardEdge& item : clipboard.edges)
      {
         const auto fromFound = localToNew.find(item.fromLocalId);
         const auto toFound = localToNew.find(item.toLocalId);
         if ((fromFound == localToNew.end()) || (toFound == localToNew.end()))
         {
            continue;
         }
         pDocument->Connect(fromFound->second,
                            item.fromPort,
                            toFound->second,
                            item.toPort,
                            nullptr);
      }

      return true;
   }
} // namespace Cgen
