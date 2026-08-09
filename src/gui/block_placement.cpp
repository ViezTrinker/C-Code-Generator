/*!
 *\file block_placement.cpp
 *\brief Non-overlapping block placement helpers.
 */
#include "gui/block_placement.h"

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

   bool BlockPlacementOverlapsExisting(WorldPosition topLeft,
                                       const std::vector<Node>& nodes)
   {
      const float paddedX = topLeft.x - BlockOverlapPadding;
      const float paddedY = topLeft.y - BlockOverlapPadding;
      const float paddedWidth = BlockNodeWidth + (BlockOverlapPadding * 2.0f);
      const float paddedHeight = BlockNodeHeight + (BlockOverlapPadding * 2.0f);

      for (size_t index = 0; index < nodes.size(); ++index)
      {
         if (RectsOverlap(paddedX,
                          paddedY,
                          paddedWidth,
                          paddedHeight,
                          nodes[index].posX,
                          nodes[index].posY,
                          BlockNodeWidth,
                          BlockNodeHeight))
         {
            return true;
         }
      }
      return false;
   }

   WorldPosition FindFreeBlockWorldPosition(WorldPosition preferred,
                                            const std::vector<Node>& nodes)
   {
      constexpr uint32_t MaxColumns = 10;
      constexpr uint32_t MaxRows = 10;
      const float stepX = BlockNodeWidth + BlockPlacementGap;
      const float stepY = BlockNodeHeight + BlockPlacementGap;

      for (uint32_t row = 0; row < MaxRows; ++row)
      {
         for (uint32_t column = 0; column < MaxColumns; ++column)
         {
            WorldPosition candidate;
            candidate.x = preferred.x + (static_cast<float>(column) * stepX);
            candidate.y = preferred.y + (static_cast<float>(row) * stepY);
            if (!BlockPlacementOverlapsExisting(candidate, nodes))
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
