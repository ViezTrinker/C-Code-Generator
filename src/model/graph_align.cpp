/*!
 *\file graph_align.cpp
 *\brief Snap-to-grid and selection alignment helpers.
 */
#include "model/graph_align.h"

#include <cmath>

#include "gui/block_placement.h"

namespace Cgen
{
   namespace
   {
      Node* FindMutableById(std::vector<Node>* pNodes, NodeId nodeId)
      {
         if (pNodes == nullptr)
         {
            return nullptr;
         }
         for (size_t index = 0; index < pNodes->size(); ++index)
         {
            if ((*pNodes)[index].id == nodeId)
            {
               return &(*pNodes)[index];
            }
         }
         return nullptr;
      }
   } // namespace

   float SnapCoordinateToGrid(float value, float gridSize)
   {
      if (gridSize <= 0.0f)
      {
         return value;
      }
      return std::round(value / gridSize) * gridSize;
   }

   void SnapNodesToGrid(std::vector<Node>* pNodes,
                        const std::vector<NodeId>& nodeIds,
                        float gridSize)
   {
      if (pNodes == nullptr)
      {
         return;
      }
      for (size_t index = 0; index < nodeIds.size(); ++index)
      {
         Node* pNode = FindMutableById(pNodes, nodeIds[index]);
         if (pNode == nullptr)
         {
            continue;
         }
         pNode->posX = SnapCoordinateToGrid(pNode->posX, gridSize);
         pNode->posY = SnapCoordinateToGrid(pNode->posY, gridSize);
      }
   }

   void AlignNodes(std::vector<Node>* pNodes,
                   const std::vector<NodeId>& nodeIds,
                   AlignSelection align)
   {
      if ((pNodes == nullptr) || (nodeIds.size() < 2))
      {
         return;
      }

      bool hasBounds = false;
      float minX = 0.0f;
      float minY = 0.0f;
      float maxX = 0.0f;
      float maxY = 0.0f;
      float sumCenterX = 0.0f;
      float sumCenterY = 0.0f;
      uint32_t counted = 0;

      for (size_t index = 0; index < nodeIds.size(); ++index)
      {
         Node* pNode = FindMutableById(pNodes, nodeIds[index]);
         if (pNode == nullptr)
         {
            continue;
         }
         const float height = ComputeBlockNodeHeight(*pNode);
         const float width = ComputeBlockNodeWidth(*pNode);
         const float right = pNode->posX + width;
         const float bottom = pNode->posY + height;
         if (!hasBounds)
         {
            minX = pNode->posX;
            minY = pNode->posY;
            maxX = right;
            maxY = bottom;
            hasBounds = true;
         }
         else
         {
            if (pNode->posX < minX)
            {
               minX = pNode->posX;
            }
            if (pNode->posY < minY)
            {
               minY = pNode->posY;
            }
            if (right > maxX)
            {
               maxX = right;
            }
            if (bottom > maxY)
            {
               maxY = bottom;
            }
         }
         sumCenterX += pNode->posX + (width * 0.5f);
         sumCenterY += pNode->posY + (height * 0.5f);
         ++counted;
      }
      if ((!hasBounds) || (counted < 2))
      {
         return;
      }

      const float centerX = sumCenterX / static_cast<float>(counted);
      const float centerY = sumCenterY / static_cast<float>(counted);

      for (size_t index = 0; index < nodeIds.size(); ++index)
      {
         Node* pNode = FindMutableById(pNodes, nodeIds[index]);
         if (pNode == nullptr)
         {
            continue;
         }
         const float height = ComputeBlockNodeHeight(*pNode);
         const float width = ComputeBlockNodeWidth(*pNode);
         switch (align)
         {
            case AlignSelection::Left:
               pNode->posX = minX;
               break;
            case AlignSelection::Right:
               pNode->posX = maxX - width;
               break;
            case AlignSelection::Top:
               pNode->posY = minY;
               break;
            case AlignSelection::Bottom:
               pNode->posY = maxY - height;
               break;
            case AlignSelection::CenterX:
               pNode->posX = centerX - (width * 0.5f);
               break;
            case AlignSelection::CenterY:
               pNode->posY = centerY - (height * 0.5f);
               break;
         }
      }
   }
} // namespace Cgen
