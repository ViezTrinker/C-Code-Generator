/*!
 *\file canvas_view.h
 *\brief Central flowchart canvas with pan, zoom, and wiring.
 */
#ifndef CANVAS_VIEW_H
#define CANVAS_VIEW_H

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "model/graph_document.h"

namespace Cgen
{
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
       *\brief Returns the selected node id, or 0.
       */
      NodeId GetSelectedNodeId(void) const;

      /*!
       *\brief Places a block at the current mouse world position.
       *
       *\param[in] blockType Block to place.
       *\param[in] screenPoint Screen mouse position.
       */
      void PlaceBlock(BlockType blockType, sf::Vector2f screenPoint);

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
       *\brief Handles mouse wheel zoom.
       *
       *\param[in] delta Wheel delta.
       *\param[in] screenPoint Screen position.
       */
      void HandleWheel(float delta, sf::Vector2f screenPoint);

      /*!
       *\brief Deletes the selection if allowed.
       */
      void DeleteSelection(void);

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

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      GraphDocument* _pDocument = nullptr;
      NodeId _selectedNodeId = 0;
      bool _isPanning = false;
      bool _isDraggingNode = false;
      sf::Vector2f _lastScreenPoint {};
      std::optional<PortHit> _wireStart;
      sf::Vector2f _wirePreviewWorld {};
   };
} // namespace Cgen

#endif // CANVAS_VIEW_H
