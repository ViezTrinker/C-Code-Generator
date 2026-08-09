/*!
 *\file context_menu.cpp
 *\brief Context menu rendering and hit-testing.
 */
#include "gui/context_menu.h"

namespace Cgen
{
   namespace
   {
      constexpr float ItemWidth = 150.0f;
      constexpr float ItemHeight = 28.0f;
   } // namespace

   ContextMenu::ContextMenu(const sf::Font& font)
      : _pFont(&font)
   {
   }

   void ContextMenu::Close(void)
   {
      _isOpen = false;
      _items.clear();
   }

   bool ContextMenu::IsOpen(void) const
   {
      return _isOpen;
   }

   void ContextMenu::RebuildBounds(void)
   {
      const float height = static_cast<float>(_items.size()) * ItemHeight;
      _bounds = sf::FloatRect(_origin, sf::Vector2f(ItemWidth, height));
      for (size_t index = 0; index < _items.size(); ++index)
      {
         _items[index].bounds = sf::FloatRect(
            sf::Vector2f(_origin.x, _origin.y + (static_cast<float>(index) * ItemHeight)),
            sf::Vector2f(ItemWidth, ItemHeight));
      }
   }

   void ContextMenu::OpenDeleteBlock(sf::Vector2f screenPoint, NodeId nodeId)
   {
      _origin = screenPoint;
      _items.clear();
      Item item;
      item.label = "Delete Block";
      item.action = ContextMenuAction::DeleteBlock;
      item.nodeId = nodeId;
      _items.push_back(item);
      RebuildBounds();
      _isOpen = true;
   }

   void ContextMenu::OpenDeleteWire(sf::Vector2f screenPoint, EdgeId edgeId)
   {
      _origin = screenPoint;
      _items.clear();
      Item item;
      item.label = "Delete Wire";
      item.action = ContextMenuAction::DeleteWire;
      item.edgeId = edgeId;
      _items.push_back(item);
      RebuildBounds();
      _isOpen = true;
   }

   bool ContextMenu::Contains(sf::Vector2f point) const
   {
      if (!_isOpen)
      {
         return false;
      }
      return _bounds.contains(point);
   }

   ContextMenuAction ContextMenu::HitTest(sf::Vector2f point,
                                          NodeId* pOutNodeId,
                                          EdgeId* pOutEdgeId) const
   {
      if (!_isOpen)
      {
         return ContextMenuAction::None;
      }
      for (size_t index = 0; index < _items.size(); ++index)
      {
         if (!_items[index].bounds.contains(point))
         {
            continue;
         }
         if (pOutNodeId != nullptr)
         {
            *pOutNodeId = _items[index].nodeId;
         }
         if (pOutEdgeId != nullptr)
         {
            *pOutEdgeId = _items[index].edgeId;
         }
         return _items[index].action;
      }
      return ContextMenuAction::None;
   }

   void ContextMenu::Draw(sf::RenderTarget* pTarget) const
   {
      if ((!_isOpen) || (pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }

      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(45, 50, 60));
      background.setOutlineColor(sf::Color(140, 150, 170));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      for (size_t index = 0; index < _items.size(); ++index)
      {
         const Item& item = _items[index];
         sf::Text label(*_pFont, item.label, 14);
         label.setFillColor(sf::Color::White);
         label.setPosition(sf::Vector2f(item.bounds.position.x + 10.0f,
                                        item.bounds.position.y + 5.0f));
         pTarget->draw(label);
      }
   }
} // namespace Cgen
