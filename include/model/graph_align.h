/*!
 *\file graph_align.h
 *\brief Snap-to-grid and selection alignment helpers.
 */
#ifndef GRAPH_ALIGN_H
#define GRAPH_ALIGN_H

#include <cstdint>
#include <vector>

#include "model/node.h"

namespace Cgen
{
   /*!
    *\brief Default canvas grid size in world units.
    */
   inline constexpr float CanvasGridSize = 20.0f;

   /*!
    *\brief Horizontal or vertical alignment for a node selection.
    */
   enum class AlignSelection: uint8_t
   {
      Left = 0,
      Right,
      Top,
      Bottom,
      CenterX,
      CenterY
   };

   /*!
    *\brief Snaps a coordinate onto the grid.
    *
    *\param[in] value World coordinate.
    *\param[in] gridSize Grid spacing (must be > 0).
    *\return Snapped coordinate.
    */
   float SnapCoordinateToGrid(float value, float gridSize);

   /*!
    *\brief Snaps the listed nodes onto the grid.
    *
    *\param[in,out] pNodes Document node list.
    *\param[in] nodeIds Nodes to snap.
    *\param[in] gridSize Grid spacing.
    */
   void SnapNodesToGrid(std::vector<Node>* pNodes,
                        const std::vector<NodeId>& nodeIds,
                        float gridSize);

   /*!
    *\brief Aligns the listed nodes along one edge or center axis.
    *
    *\param[in,out] pNodes Document node list.
    *\param[in] nodeIds Nodes to align (needs at least two).
    *\param[in] align Alignment mode.
    */
   void AlignNodes(std::vector<Node>* pNodes,
                   const std::vector<NodeId>& nodeIds,
                   AlignSelection align);
} // namespace Cgen

#endif // GRAPH_ALIGN_H
