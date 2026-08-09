/*!
 *\file property_panel.cpp
 *\brief Property inspector implementation.
 */
#include "gui/property_panel.h"

#include <vector>

#include "model/block_type.h"

namespace Cgen
{
   namespace
   {
      constexpr float TitleHeight = 28.0f;
      constexpr float TypeLabelHeight = 18.0f;
      constexpr float HelpLineHeight = 15.0f;
      constexpr float HelpBottomGap = 10.0f;
      constexpr float FieldRowHeight = 44.0f;
      constexpr unsigned int HelpCharacterSize = 12;
   } // namespace

   PropertyPanel::PropertyPanel(const sf::Font& font)
      : _pFont(&font)
   {
   }

   void PropertyPanel::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      RebuildFields();
   }

   void PropertyPanel::SetHistory(DocumentHistory* pHistory)
   {
      _pHistory = pHistory;
   }

   void PropertyPanel::SetSelection(GraphDocument* pDocument, NodeId selectedNodeId)
   {
      if ((pDocument == _pDocument) && (selectedNodeId == _selectedNodeId))
      {
         return;
      }
      CommitActiveField();
      _pDocument = pDocument;
      _selectedNodeId = selectedNodeId;
      _activeFieldIndex = -1;
      RebuildFields();
   }

   void PropertyPanel::ReloadFromDocument(void)
   {
      _activeFieldIndex = -1;
      RebuildFields();
   }

   float PropertyPanel::FieldsStartY(void) const
   {
      float cursorY = _bounds.position.y + TitleHeight + TypeLabelHeight;
      if (!_helpLines.empty())
      {
         cursorY += static_cast<float>(_helpLines.size()) * HelpLineHeight;
         cursorY += HelpBottomGap;
      }
      return cursorY;
   }

   void PropertyPanel::RebuildHelpLines(std::string_view helpText)
   {
      _helpLines.clear();
      if ((_pFont == nullptr) || helpText.empty())
      {
         return;
      }

      const float maxWidth = _bounds.size.x - 16.0f;
      if (maxWidth < 20.0f)
      {
         _helpLines.push_back(std::string(helpText));
         return;
      }

      std::string currentLine;
      std::string currentWord;
      for (size_t index = 0; index <= helpText.size(); ++index)
      {
         const bool atEnd = (index == helpText.size());
         const char character = atEnd ? ' ' : helpText[index];
         if ((character == ' ') || (character == '\n') || atEnd)
         {
            if (!currentWord.empty())
            {
               std::string candidate = currentLine;
               if (!candidate.empty())
               {
                  candidate.push_back(' ');
               }
               candidate.append(currentWord);

               sf::Text measure(*_pFont, candidate, HelpCharacterSize);
               if ((measure.getLocalBounds().size.x > maxWidth) && (!currentLine.empty()))
               {
                  _helpLines.push_back(currentLine);
                  currentLine = currentWord;
               }
               else
               {
                  currentLine = candidate;
               }
               currentWord.clear();
            }
            if (character == '\n')
            {
               _helpLines.push_back(currentLine);
               currentLine.clear();
            }
         }
         else
         {
            currentWord.push_back(character);
         }
      }
      if (!currentLine.empty())
      {
         _helpLines.push_back(currentLine);
      }
   }

   void PropertyPanel::RebuildFields(void)
   {
      _fields.clear();
      _helpLines.clear();
      if ((_pDocument == nullptr) || (_selectedNodeId == 0))
      {
         return;
      }
      const Node* pNode = _pDocument->FindNode(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }

      RebuildHelpLines(BlockTypeHelpText(pNode->type));

      float cursorY = FieldsStartY();
      for (PropertyMap::const_iterator iterator = pNode->properties.begin();
           iterator != pNode->properties.end();
           ++iterator)
      {
         Field field;
         field.key = iterator->first;
         field.value = iterator->second;
         field.bounds =
            sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, cursorY + 16.0f),
                          sf::Vector2f(_bounds.size.x - 16.0f, 22.0f));
         _fields.push_back(field);
         cursorY += FieldRowHeight;
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
      const auto found = pNode->properties.find(field.key);
      if ((found != pNode->properties.end()) && (found->second == field.value))
      {
         return;
      }
      if (_pHistory != nullptr)
      {
         _pHistory->PushCheckpoint(*_pDocument);
      }
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
      if ((keyCode == sf::Keyboard::Key::Backspace) ||
          (keyCode == sf::Keyboard::Key::Delete))
      {
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
                                        _bounds.position.y + TitleHeight));
         pTarget->draw(empty);
         return;
      }

      const Node* pNode = _pDocument->FindNode(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }

      sf::Text typeLabel(*_pFont,
                         std::string("Type: ") + std::string(BlockTypeLabel(pNode->type)),
                         13);
      typeLabel.setFillColor(sf::Color(200, 200, 200));
      typeLabel.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                         _bounds.position.y + TitleHeight));
      pTarget->draw(typeLabel);

      float helpY = _bounds.position.y + TitleHeight + TypeLabelHeight;
      for (size_t lineIndex = 0; lineIndex < _helpLines.size(); ++lineIndex)
      {
         sf::Text helpLine(*_pFont, _helpLines[lineIndex], HelpCharacterSize);
         helpLine.setFillColor(sf::Color(150, 170, 150));
         helpLine.setPosition(sf::Vector2f(_bounds.position.x + 8.0f, helpY));
         pTarget->draw(helpLine);
         helpY += HelpLineHeight;
      }

      float labelY = FieldsStartY();
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
         labelY += FieldRowHeight;
      }
   }
} // namespace Cgen
