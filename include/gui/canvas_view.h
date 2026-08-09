/*!
 *\file canvas_view.h
 *\brief Central flowchart canvas with pan, zoom, and wiring.
 */
#ifndef CANVAS_VIEW_H
#define CANVAS_VIEW_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "gui/document_history.h"
#include "model/graph_clipboard.h"
#include "model/graph_document.h"

namespace Cgen
{
   /*!
    *\brief Kind of object under a canvas query point.
    */
   enum class CanvasHitKind: uint8_t
   {
      None = 0,
      Empty,
      Node,
      Wire
   };

   /*!
    *\brief Result of querying the canvas under the cursor.
    */
   struct CanvasHitInfo
   {
      CanvasHitKind kind = CanvasHitKind::None;
      NodeId nodeId = 0;
      EdgeId edgeId = 0;
   };

   /*!
    *\brief Interactive graph canvas.
    */
   class CanvasView
   {
   public:
      /*!
       *\brief Constructs the canvas.
       *
       *\param[in] font Font for node labels.
       */
      explicit CanvasView(const sf::Font& font);

      /*!
       *\brief Sets the screen rectangle used by the canvas.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Binds the document to edit.
       *
       *\param[in,out] pDocument Document pointer.
       */
      void SetDocument(GraphDocument* pDocument);

      /*!
       *\brief Binds the undo history used before edits.
       *
       *\param[in,out] pHistory History pointer, or nullptr.
       */
      void SetHistory(DocumentHistory* pHistory);

      /*!
       *\brief Returns true if the point lies inside the canvas bounds.
       *
       *\param[in] point Screen position.
       */
      bool Contains(sf::Vector2f point) const;

      /*!
       *\brief Returns the primary selected node id, or 0.
       */
      NodeId GetSelectedNodeId(void) const;

      /*!
       *\brief Returns all selected node ids.
       */
      const std::vector<NodeId>& GetSelectedNodeIds(void) const;

      /*!
       *\brief Replaces the selection with a single node id.
       *
       *\param[in] nodeId Node id, or 0 to clear.
       */
      void SetSelectedNodeId(NodeId nodeId);

      /*!
       *\brief Replaces the multi-selection.
       *
       *\param[in] nodeIds Selected ids (empty clears).
       */
      void SetSelectedNodeIds(const std::vector<NodeId>& nodeIds);

      /*!
       *\brief Centers the viewport on a node.
       *
       *\param[in] nodeId Node to center.
       */
      void CenterOnNode(NodeId nodeId);

      /*!
       *\brief Zooms and pans so all nodes fit in the canvas.
       */
      void FitAllNodes(void);

      /*!
       *\brief Zooms and pans so the current selection fits (or all if empty).
       */
      void FitSelection(void);

      /*!
       *\brief Selects every node in the document.
       */
      void SelectAll(void);

      /*!
       *\brief Copies the current selection into the in-app clipboard.
       */
      void CopySelection(void);

      /*!
       *\brief Pastes the in-app clipboard with an offset.
       */
      void PasteClipboard(void);

      /*!
       *\brief Applies auto-layout with one undo checkpoint.
       */
      void TidyLayout(void);

      /*!
       *\brief Places a block at the current mouse world position.
       *
       *\param[in] blockType Block to place.
       *\param[in] screenPoint Screen mouse position.
       */
      void PlaceBlock(BlockType blockType, sf::Vector2f screenPoint);

      /*!
       *\brief Queries what lies under a screen point.
       *
       *\param[in] screenPoint Screen position.
       *\param[out] pOutHit Hit details.
       *\return true if the point is inside the canvas.
       */
      bool QueryHit(sf::Vector2f screenPoint, CanvasHitInfo* pOutHit) const;

      /*!
       *\brief Handles mouse button press.
       *
       *\param[in] button Mouse button.
       *\param[in] screenPoint Screen position.
       *\return true if handled.
       */
      bool HandleMousePress(sf::Mouse::Button button, sf::Vector2f screenPoint);

      /*!
       *\brief Handles mouse button release.
       *
       *\param[in] button Mouse button.
       *\param[in] screenPoint Screen position.
       *\return true if handled.
       */
      bool HandleMouseRelease(sf::Mouse::Button button, sf::Vector2f screenPoint);

      /*!
       *\brief Handles mouse move.
       *
       *\param[in] screenPoint Screen position.
       */
      void HandleMouseMove(sf::Vector2f screenPoint);

      /*!
       *\brief Handles mouse wheel pan or zoom.
       *
       *\param[in] delta Wheel delta (sign indicates direction).
       *\param[in] screenPoint Screen position.
       *\param[in] horizontal true for a horizontal wheel/trackpad axis.
       */
      void HandleWheel(float delta, sf::Vector2f screenPoint, bool horizontal);

      /*!
       *\brief Pans the viewport with arrow keys when the canvas can accept them.
       *
       *\param[in] keyCode Pressed key.
       *\return true if the key was consumed as a pan.
       */
      bool HandlePanKey(sf::Keyboard::Key keyCode);

      /*!
       *\brief Deletes the selection if allowed.
       */
      void DeleteSelection(void);

      /*!
       *\brief Removes an edge by id with an undo checkpoint.
       *
       *\param[in] edgeId Edge to remove.
       */
      void DeleteEdge(EdgeId edgeId);

      /*!
       *\brief Removes a node by id with an undo checkpoint.
       *
       *\param[in] nodeId Node to remove.
       */
      void DeleteNode(NodeId nodeId);

      /*!
       *\brief Draws the canvas.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      struct PortHit
      {
         NodeId nodeId = 0;
         std::string portName;
         sf::Vector2f worldPosition {};
         PortKind kind = PortKind::Control;
         PortDirection direction = PortDirection::Out;
      };

      sf::Vector2f ScreenToWorld(sf::Vector2f screenPoint) const;
      sf::Vector2f WorldToScreen(sf::Vector2f worldPoint) const;
      sf::FloatRect NodeBounds(const Node& node) const;
      sf::Vector2f PortWorldPosition(const Node& node, size_t portIndex) const;
      bool HitTestPort(sf::Vector2f worldPoint, PortHit* pOutHit) const;
      NodeId HitTestNode(sf::Vector2f worldPoint) const;
      void PushCheckpoint(void);
      bool IsNodeSelected(NodeId nodeId) const;
      void ClearSelection(void);
      void AddToSelection(NodeId nodeId);
      void ToggleSelection(NodeId nodeId);
      void RemoveFromSelection(NodeId nodeId);
      void PanByScreenDelta(float screenDeltaX, float screenDeltaY);
      void ZoomAtScreenPoint(float delta, sf::Vector2f screenPoint);
      void DrawFunctionRegions(sf::RenderTarget* pTarget) const;
      void DrawStickyFunctionHeaders(sf::RenderTarget* pTarget) const;
      void DrawMinimap(sf::RenderTarget* pTarget) const;
      sf::FloatRect MinimapScreenRect(void) const;
      bool HandleMinimapPress(sf::Vector2f screenPoint);
      void FitNodes(const std::vector<NodeId>& nodeIds);
      void CollectFunctionBodyBounds(NodeId functionId,
                                    float* pOutMinX,
                                    float* pOutMinY,
                                    float* pOutMaxX,
                                    float* pOutMaxY) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      GraphDocument* _pDocument = nullptr;
      DocumentHistory* _pHistory = nullptr;
      std::vector<NodeId> _selectedNodeIds;
      GraphClipboard _clipboard;
      uint32_t _pasteCascade = 0;
      bool _isPanning = false;
      bool _isDraggingNode = false;
      bool _isMarquee = false;
      bool _dragCheckpointTaken = false;
      bool _isMinimapDragging = false;
      sf::Vector2f _lastScreenPoint {};
      sf::Vector2f _marqueeStartWorld {};
      sf::Vector2f _marqueeEndWorld {};
      std::optional<PortHit> _wireStart;
      sf::Vector2f _wirePreviewWorld {};
      bool _hasHoveredPort = false;
      std::string _hoveredPortName;
      sf::Vector2f _hoveredPortScreen {};
   };
} // namespace Cgen

#endif // CANVAS_VIEW_H
