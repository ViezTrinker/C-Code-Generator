/*!
 *\file graph_layout.cpp
 *\brief Layered control-flow auto-layout and expression nudging.
 */
#include "model/graph_layout.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "model/block_type.h"
#include "model/edge.h"
#include "model/port.h"

namespace Cgen
{
   namespace
   {
      constexpr float LayoutOriginX = 80.0f;
      constexpr float LayoutOriginY = 80.0f;
      constexpr float LayoutHSpacing = 180.0f;
      constexpr float LayoutVSpacing = 90.0f;
      constexpr float FunctionStackGap = 320.0f;
      constexpr float ExpressionOffsetX = -160.0f;
      constexpr float ExpressionOffsetY = 70.0f;

      void LayoutFromRoot(GraphDocument* pDocument,
                          NodeId rootId,
                          float originX,
                          float originY,
                          std::unordered_set<NodeId>* pPlaced)
      {
         if ((pDocument == nullptr) || (pPlaced == nullptr) || (rootId == 0))
         {
            return;
         }

         std::unordered_map<NodeId, int32_t> depthById;
         std::unordered_map<NodeId, int32_t> rowById;
         std::unordered_map<int32_t, int32_t> nextRowAtDepth;
         std::queue<NodeId> pending;

         depthById[rootId] = 0;
         rowById[rootId] = 0;
         nextRowAtDepth[0] = 1;
         pending.push(rootId);

         while (!pending.empty())
         {
            const NodeId currentId = pending.front();
            pending.pop();
            const Node* pNode = pDocument->FindNode(currentId);
            if (pNode == nullptr)
            {
               continue;
            }
            const int32_t depth = depthById[currentId];
            for (const Port& port : pNode->ports)
            {
               if ((port.kind != PortKind::Control) ||
                   (port.direction != PortDirection::Out))
               {
                  continue;
               }
               const Edge* pEdge = pDocument->FindOutgoingEdge(currentId, port.name);
               if (pEdge == nullptr)
               {
                  continue;
               }
               if (depthById.find(pEdge->toNodeId) != depthById.end())
               {
                  continue;
               }
               const int32_t childDepth = depth + 1;
               const int32_t childRow = nextRowAtDepth[childDepth];
               ++nextRowAtDepth[childDepth];
               depthById[pEdge->toNodeId] = childDepth;
               rowById[pEdge->toNodeId] = childRow;
               pending.push(pEdge->toNodeId);
            }
         }

         for (const auto& pair : depthById)
         {
            Node* pMutable = pDocument->FindNodeMutable(pair.first);
            if (pMutable == nullptr)
            {
               continue;
            }
            const int32_t depth = pair.second;
            const int32_t row = rowById[pair.first];
            pMutable->posX = originX + (static_cast<float>(depth) * LayoutHSpacing);
            pMutable->posY = originY + (static_cast<float>(row) * LayoutVSpacing);
            pPlaced->insert(pair.first);
         }
      }

      void NudgeExpressionFeeders(GraphDocument* pDocument,
                                  const std::unordered_set<NodeId>& placed)
      {
         if (pDocument == nullptr)
         {
            return;
         }
         for (const Edge& edge : pDocument->GetEdges())
         {
            const Node* pFrom = pDocument->FindNode(edge.fromNodeId);
            const Node* pTo = pDocument->FindNode(edge.toNodeId);
            if ((pFrom == nullptr) || (pTo == nullptr))
            {
               continue;
            }
            if (!IsExpressionBlock(pFrom->type))
            {
               continue;
            }
            if (placed.find(pTo->id) == placed.end())
            {
               continue;
            }
            if (placed.find(pFrom->id) != placed.end())
            {
               continue;
            }
            Node* pMutable = pDocument->FindNodeMutable(pFrom->id);
            if (pMutable == nullptr)
            {
               continue;
            }
            pMutable->posX = pTo->posX + ExpressionOffsetX;
            pMutable->posY = pTo->posY + ExpressionOffsetY;
         }
      }
   } // namespace

   void ApplyAutoLayout(GraphDocument* pDocument)
   {
      if (pDocument == nullptr)
      {
         return;
      }

      std::unordered_set<NodeId> placed;
      float stackY = LayoutOriginY;

      for (const Node& node : pDocument->GetNodes())
      {
         if (node.type == BlockType::Start)
         {
            LayoutFromRoot(pDocument, node.id, LayoutOriginX, stackY, &placed);
            stackY += FunctionStackGap;
            break;
         }
      }

      for (const Node& node : pDocument->GetNodes())
      {
         if (node.type != BlockType::FunctionDef)
         {
            continue;
         }
         LayoutFromRoot(pDocument, node.id, LayoutOriginX, stackY, &placed);
         stackY += FunctionStackGap;
      }

      NudgeExpressionFeeders(pDocument, placed);
      pDocument->SetDirty(true);
   }
} // namespace Cgen
