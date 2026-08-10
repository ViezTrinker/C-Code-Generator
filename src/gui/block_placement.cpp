/*!
 *\file block_placement.cpp
 *\brief Non-overlapping block placement helpers.
 */
#include "gui/block_placement.h"

#include "model/block_type.h"

namespace Cgen
{
   namespace
   {
      bool RangesOverlap(float startA, float sizeA, float startB, float sizeB)
      {
         const float endA = startA + sizeA;
         const float endB = startB + sizeB;
         return (startA < endB) && (startB < endA);
      }

      bool RectsOverlap(float leftX,
                        float leftY,
                        float leftWidth,
                        float leftHeight,
                        float rightX,
                        float rightY,
                        float rightWidth,
                        float rightHeight)
      {
         return RangesOverlap(leftX, leftWidth, rightX, rightWidth) &&
                RangesOverlap(leftY, leftHeight, rightY, rightHeight);
      }
   } // namespace

   float BlockFirstPortOffsetY(const Node& node)
   {
      float offset = BlockPortTopOffset;
      if (node.type == BlockType::FunctionDef)
      {
         offset += BlockFunctionHeaderExtra;
      }
      return offset;
   }

   float ComputeBlockNodeHeight(const Node& node)
   {
      size_t inCount = 0;
      size_t outCount = 0;
      for (size_t index = 0; index < node.ports.size(); ++index)
      {
         if (!node.ports[index].visible)
         {
            continue;
         }
         if (node.ports[index].direction == PortDirection::In)
         {
            ++inCount;
         }
         else
         {
            ++outCount;
         }
      }

      const size_t sideCount = (inCount > outCount) ? inCount : outCount;
      if (sideCount == 0)
      {
         if (node.type == BlockType::FunctionDef)
         {
            const float headerHeight =
               BlockTitleBandHeight + BlockFunctionHeaderExtra + BlockPortBottomPad;
            if (headerHeight > BlockNodeHeight)
            {
               return headerHeight;
            }
         }
         return BlockNodeHeight;
      }

      const float fitted =
         BlockFirstPortOffsetY(node) +
         (static_cast<float>(sideCount - 1) * BlockPortSpacing) +
         BlockPortBottomPad;
      if (fitted < BlockNodeHeight)
      {
         return BlockNodeHeight;
      }
      return fitted;
   }

   bool BlockPlacementOverlapsExisting(WorldPosition topLeft,
                                       const std::vector<Node>& nodes,
                                       float proposedHeight)
   {
      const float paddedX = topLeft.x - BlockOverlapPadding;
      const float paddedY = topLeft.y - BlockOverlapPadding;
      const float paddedWidth = BlockNodeWidth + (BlockOverlapPadding * 2.0f);
      const float paddedHeight = proposedHeight + (BlockOverlapPadding * 2.0f);

      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const float existingHeight = ComputeBlockNodeHeight(nodes[index]);
         if (RectsOverlap(paddedX,
                          paddedY,
                          paddedWidth,
                          paddedHeight,
                          nodes[index].posX,
                          nodes[index].posY,
                          BlockNodeWidth,
                          existingHeight))
         {
            return true;
         }
      }
      return false;
   }

   WorldPosition FindFreeBlockWorldPosition(WorldPosition preferred,
                                            const std::vector<Node>& nodes,
                                            float proposedHeight)
   {
      constexpr uint32_t MaxColumns = 10;
      constexpr uint32_t MaxRows = 10;
      const float stepX = BlockNodeWidth + BlockPlacementGap;
      const float stepY = proposedHeight + BlockPlacementGap;

      for (uint32_t row = 0; row < MaxRows; ++row)
      {
         for (uint32_t column = 0; column < MaxColumns; ++column)
         {
            WorldPosition candidate;
            candidate.x = preferred.x + (static_cast<float>(column) * stepX);
            candidate.y = preferred.y + (static_cast<float>(row) * stepY);
            if (!BlockPlacementOverlapsExisting(candidate, nodes, proposedHeight))
            {
               return candidate;
            }
         }
      }

      WorldPosition fallback;
      fallback.x = preferred.x + (static_cast<float>(MaxColumns) * stepX);
      fallback.y = preferred.y + (static_cast<float>(MaxRows) * stepY);
      return fallback;
   }
} // namespace Cgen
