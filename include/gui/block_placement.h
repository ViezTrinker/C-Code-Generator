/*!
 *\file block_placement.h
 *\brief Finds non-overlapping world positions for newly placed blocks.
 */
#ifndef BLOCK_PLACEMENT_H
#define BLOCK_PLACEMENT_H

#include <cstdint>
#include <vector>

#include "model/node.h"

namespace Cgen
{
   inline constexpr float BlockNodeWidth = 140.0f;
   inline constexpr float BlockNodeHeight = 56.0f;
   inline constexpr float BlockPlacementGap = 16.0f;
   inline constexpr float BlockOverlapPadding = 12.0f;

   /*!
    *\brief World-space top-left position.
    */
   struct WorldPosition
   {
      float x = 0.0f;
      float y = 0.0f;
   };

   /*!
    *\brief Returns true if a block at topLeft would overlap any existing node.
    *
    *\param[in] topLeft Proposed block top-left.
    *\param[in] nodes Existing nodes on the canvas.
    */
   bool BlockPlacementOverlapsExisting(WorldPosition topLeft,
                                       const std::vector<Node>& nodes);

   /*!
    *\brief Finds a free top-left near preferred that does not overlap nodes.
    *
    *\param[in] preferred Preferred top-left position.
    *\param[in] nodes Existing nodes on the canvas.
    *\return Free world position (preferred if already free).
    */
   WorldPosition FindFreeBlockWorldPosition(WorldPosition preferred,
                                            const std::vector<Node>& nodes);
} // namespace Cgen

#endif // BLOCK_PLACEMENT_H
