/*!
 *\file context_menu.h
 *\brief Simple right-click popup menu for canvas actions.
 */
#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include <cstdint>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "gui/ui_theme.h"
#include "model/edge.h"
#include "model/node.h"

namespace Cgen
{
   /*!
    *\brief Action chosen from the context menu.
    */
   enum class ContextMenuAction: uint8_t
   {
      None = 0,
      DeleteBlock,
      DeleteWire
   };

   /*!
    *\brief Floating context menu with a short item list.
    */
   class ContextMenu
   {
   public:
      /*!
       *\brief Constructs the menu.
       *
       *\param[in] font Font for labels.
       */
      explicit ContextMenu(const sf::Font& font);

      /*!
       *\brief Applies a UI color theme.
       *
       *\param[in] theme Theme palette.
       */
      void SetTheme(const UiTheme& theme);

      /*!
       *\brief Closes the menu if open.
       */
      void Close(void);

      /*!
       *\brief Returns true when the menu is visible.
       */
      bool IsOpen(void) const;

      /*!
       *\brief Opens a delete-block menu for a node.
       *
       *\param[in] screenPoint Anchor position.
       *\param[in] nodeId Target node.
       */
      void OpenDeleteBlock(sf::Vector2f screenPoint, NodeId nodeId);

      /*!
       *\brief Opens a delete-wire menu for an edge.
       *
       *\param[in] screenPoint Anchor position.
       *\param[in] edgeId Target edge.
       */
      void OpenDeleteWire(sf::Vector2f screenPoint, EdgeId edgeId);

      /*!
       *\brief Returns true if the point lies inside the menu.
       *
       *\param[in] point Mouse position.
       */
      bool Contains(sf::Vector2f point) const;

      /*!
       *\brief Hit-tests a click and returns the chosen action.
       *
       *\param[in] point Mouse position.
       *\param[out] pOutNodeId Node id for DeleteBlock.
       *\param[out] pOutEdgeId Edge id for DeleteWire.
       *\return Selected action, or None.
       */
      ContextMenuAction HitTest(sf::Vector2f point,
                                NodeId* pOutNodeId,
                                EdgeId* pOutEdgeId) const;

      /*!
       *\brief Draws the menu when open.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      struct Item
      {
         std::string label;
         ContextMenuAction action = ContextMenuAction::None;
         NodeId nodeId = 0;
         EdgeId edgeId = 0;
         sf::FloatRect bounds {};
      };

      void RebuildBounds(void);

      const sf::Font* _pFont = nullptr;
      bool _isOpen = false;
      sf::Vector2f _origin {};
      sf::FloatRect _bounds {};
      std::vector<Item> _items;
      UiTheme _theme = GetUiTheme(UiThemeId::Dark);
   };
} // namespace Cgen

#endif // CONTEXT_MENU_H
