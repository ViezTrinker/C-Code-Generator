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
   inline constexpr float BlockPortTopOffset = 18.0f;
   inline constexpr float BlockPortSpacing = 14.0f;
   inline constexpr float BlockPortBottomPad = 12.0f;
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
    *\brief Computes node body height so ports stay inside the block shape.
    *
    *\param[in] node Node whose ports determine height.
    *\return Height in world units (at least BlockNodeHeight).
    */
   float ComputeBlockNodeHeight(const Node& node);

   /*!
    *\brief Returns true if a block at topLeft would overlap any existing node.
    *
    *\param[in] topLeft Proposed block top-left.
    *\param[in] nodes Existing nodes on the canvas.
    *\param[in] proposedHeight Height of the block being placed.
    */
   bool BlockPlacementOverlapsExisting(WorldPosition topLeft,
                                       const std::vector<Node>& nodes,
                                       float proposedHeight = BlockNodeHeight);

   /*!
    *\brief Finds a free top-left near preferred that does not overlap nodes.
    *
    *\param[in] preferred Preferred top-left position.
    *\param[in] nodes Existing nodes on the canvas.
    *\param[in] proposedHeight Height of the block being placed.
    *\return Free world position (preferred if already free).
    */
   WorldPosition FindFreeBlockWorldPosition(WorldPosition preferred,
                                            const std::vector<Node>& nodes,
                                            float proposedHeight = BlockNodeHeight);
} // namespace Cgen

#endif // BLOCK_PLACEMENT_H
