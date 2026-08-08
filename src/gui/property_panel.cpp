/*!
 *\file property_panel.cpp
 *\brief Property inspector implementation.
 */
#include "gui/property_panel.h"

#include <vector>

namespace Cgen
{
   PropertyPanel::PropertyPanel(const sf::Font& font)
      : _pFont(&font)
   {
   }

   void PropertyPanel::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      RebuildFields();
   }

   void PropertyPanel::SetSelection(GraphDocument* pDocument, NodeId selectedNodeId)
   {
      CommitActiveField();
      _pDocument = pDocument;
      _selectedNodeId = selectedNodeId;
      _activeFieldIndex = -1;
      RebuildFields();
   }

   void PropertyPanel::RebuildFields(void)
   {
      _fields.clear();
      if ((_pDocument == nullptr) || (_selectedNodeId == 0))
      {
         return;
      }
      const Node* pNode = _pDocument->FindNode(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }
      constexpr float RowHeight = 44.0f;
      float cursorY = _bounds.position.y + 36.0f;
      for (const auto& entry : pNode->properties)
      {
         Field field;
         field.key = entry.first;
         field.value = entry.second;
         field.bounds =
            sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, cursorY + 16.0f),
                          sf::Vector2f(_bounds.size.x - 16.0f, 22.0f));
         _fields.push_back(field);
         cursorY += RowHeight;
      }
   }

   void PropertyPanel::CommitActiveField(void)
   {
      if ((_activeFieldIndex < 0) || (_pDocument == nullptr) ||
          (_selectedNodeId == 0))
      {
         return;
      }
      if (static_cast<size_t>(_activeFieldIndex) >= _fields.size())
      {
         return;
      }
      Node* pNode = _pDocument->FindNodeMutable(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }
      const Field& field = _fields[static_cast<size_t>(_activeFieldIndex)];
      pNode->properties[field.key] = field.value;
      _pDocument->SetDirty(true);
   }

   bool PropertyPanel::HandleClick(sf::Vector2f point)
   {
      if (!_bounds.contains(point))
      {
         return false;
      }
      CommitActiveField();
      _activeFieldIndex = -1;
      for (size_t index = 0; index < _fields.size(); ++index)
      {
         if (_fields[index].bounds.contains(point))
         {
            _activeFieldIndex = static_cast<int32_t>(index);
            return true;
         }
      }
      return true;
   }

   bool PropertyPanel::HandleTextEntered(uint32_t unicode)
   {
      if (_activeFieldIndex < 0)
      {
         return false;
      }
      if (static_cast<size_t>(_activeFieldIndex) >= _fields.size())
      {
         return false;
      }
      if ((unicode == 8) || (unicode == 127))
      {
         std::string& value = _fields[static_cast<size_t>(_activeFieldIndex)].value;
         if (!value.empty())
         {
            value.pop_back();
         }
         return true;
      }
      if ((unicode >= 32) && (unicode < 127))
      {
         _fields[static_cast<size_t>(_activeFieldIndex)].value.push_back(
            static_cast<char>(unicode));
         return true;
      }
      return false;
   }

   bool PropertyPanel::HandleKey(sf::Keyboard::Key keyCode)
   {
      if (_activeFieldIndex < 0)
      {
         return false;
      }
      if (keyCode == sf::Keyboard::Key::Enter)
      {
         CommitActiveField();
         _activeFieldIndex = -1;
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Escape)
      {
         RebuildFields();
         _activeFieldIndex = -1;
         return true;
      }
      return false;
   }

   void PropertyPanel::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }
      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(35, 38, 44));
      pTarget->draw(background);

      sf::Text title(*_pFont, "Properties", 16);
      title.setFillColor(sf::Color::White);
      title.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                     _bounds.position.y + 6.0f));
      pTarget->draw(title);

      if ((_pDocument == nullptr) || (_selectedNodeId == 0))
      {
         sf::Text empty(*_pFont, "No selection", 13);
         empty.setFillColor(sf::Color(160, 160, 160));
         empty.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                        _bounds.position.y + 36.0f));
         pTarget->draw(empty);
         return;
      }

      const Node* pNode = _pDocument->FindNode(_selectedNodeId);
      if (pNode != nullptr)
      {
         sf::Text typeLabel(*_pFont,
                            std::string("Type: ") + std::string(BlockTypeLabel(pNode->type)),
                            13);
         typeLabel.setFillColor(sf::Color(200, 200, 200));
         typeLabel.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                            _bounds.position.y + 28.0f));
         pTarget->draw(typeLabel);
      }

      float labelY = _bounds.position.y + 36.0f;
      for (size_t index = 0; index < _fields.size(); ++index)
      {
         const Field& field = _fields[index];
         sf::Text keyText(*_pFont, field.key, 12);
         keyText.setFillColor(sf::Color(180, 180, 180));
         keyText.setPosition(sf::Vector2f(_bounds.position.x + 8.0f, labelY));
         pTarget->draw(keyText);

         sf::RectangleShape box;
         box.setPosition(field.bounds.position);
         box.setSize(field.bounds.size);
         if (static_cast<int32_t>(index) == _activeFieldIndex)
         {
            box.setFillColor(sf::Color(70, 80, 100));
         }
         else
         {
            box.setFillColor(sf::Color(50, 54, 62));
         }
         box.setOutlineColor(sf::Color(90, 100, 120));
         box.setOutlineThickness(1.0f);
         pTarget->draw(box);

         sf::Text valueText(*_pFont, field.value, 13);
         valueText.setFillColor(sf::Color::White);
         valueText.setPosition(sf::Vector2f(field.bounds.position.x + 4.0f,
                                            field.bounds.position.y + 2.0f));
         pTarget->draw(valueText);
         labelY += 44.0f;
      }
   }
} // namespace Cgen
