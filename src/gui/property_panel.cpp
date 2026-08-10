/*!
 *\file property_panel.cpp
 *\brief Property inspector implementation.
 */
#include "gui/property_panel.h"

#include <algorithm>
#include <vector>

#include "codegen/c_codegen.h"
#include "model/block_type.h"

namespace Cgen
{
   namespace
   {
      constexpr float TitleHeight = 28.0f;
      constexpr float TypeLabelHeight = 18.0f;
      constexpr float HelpLineHeight = 15.0f;
      constexpr float HelpBottomGap = 6.0f;
      constexpr float PreviewLineHeight = 15.0f;
      constexpr float PreviewBottomGap = 10.0f;
      constexpr float FieldRowHeight = 44.0f;
      constexpr float ChoiceItemHeight = 22.0f;
      constexpr float ChoicePopupMaxHeight = 242.0f;
      constexpr unsigned int HelpCharacterSize = 12;
      constexpr unsigned int PreviewCharacterSize = 12;

      bool IsTypePropertyKey(std::string_view key)
      {
         if ((key == "type") || (key == "returnType") || (key == "toType") ||
             (key == "elemType"))
         {
            return true;
         }
         return (key.size() > 4) && (key.rfind("Type") == (key.size() - 4)) &&
                (key.find("param") == 0);
      }

      bool ShouldHidePropertyKey(std::string_view key)
      {
         return (key == "collapsed") || (key == "params");
      }

      void AppendUniqueChoice(std::vector<std::string>* pOut, std::string_view value)
      {
         if ((pOut == nullptr) || value.empty())
         {
            return;
         }
         for (size_t index = 0; index < pOut->size(); ++index)
         {
            if ((*pOut)[index] == value)
            {
               return;
            }
         }
         pOut->push_back(std::string(value));
      }

      void AppendParamCountChoices(std::vector<std::string>* pOut)
      {
         for (uint32_t index = 0; index <= MaxFunctionParams; ++index)
         {
            AppendUniqueChoice(pOut, std::to_string(index));
         }
      }

      void AppendYesNoChoices(std::vector<std::string>* pOut)
      {
         AppendUniqueChoice(pOut, "0");
         AppendUniqueChoice(pOut, "1");
      }

      void AppendPrimitiveTypeChoices(std::vector<std::string>* pOut)
      {
         if (pOut == nullptr)
         {
            return;
         }
         constexpr const char* BaseTypes[] = {
            "void",     "bool",     "char",     "int8_t",   "uint8_t",  "int16_t",
            "uint16_t", "int32_t",  "uint32_t", "int64_t",  "uint64_t", "float",
            "double",   "size_t",   "FILE"
         };
         constexpr size_t BaseTypeCount = sizeof(BaseTypes) / sizeof(BaseTypes[0]);
         for (size_t index = 0; index < BaseTypeCount; ++index)
         {
            AppendUniqueChoice(pOut, BaseTypes[index]);
         }
         for (size_t index = 0; index < BaseTypeCount; ++index)
         {
            std::string pointerType = BaseTypes[index];
            pointerType.push_back('*');
            AppendUniqueChoice(pOut, pointerType);
         }
      }

      void AppendNamedTypesFromDocument(const GraphDocument& document,
                                        std::vector<std::string>* pOut)
      {
         if (pOut == nullptr)
         {
            return;
         }
         const std::vector<Node>& nodes = document.GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            if ((nodes[index].type != BlockType::StructDecl) &&
                (nodes[index].type != BlockType::EnumDecl) &&
                (nodes[index].type != BlockType::TypedefDecl))
            {
               continue;
            }
            const auto nameIterator = nodes[index].properties.find("name");
            if ((nameIterator == nodes[index].properties.end()) ||
                nameIterator->second.empty())
            {
               continue;
            }
            AppendUniqueChoice(pOut, nameIterator->second);
            std::string pointerType = nameIterator->second;
            pointerType.push_back('*');
            AppendUniqueChoice(pOut, pointerType);
         }
      }

      void AppendCompoundOpChoices(std::vector<std::string>* pOut)
      {
         AppendUniqueChoice(pOut, "+");
         AppendUniqueChoice(pOut, "-");
         AppendUniqueChoice(pOut, "*");
         AppendUniqueChoice(pOut, "/");
         AppendUniqueChoice(pOut, "%");
      }

      void AppendAccessChoices(std::vector<std::string>* pOut)
      {
         AppendUniqueChoice(pOut, ".");
         AppendUniqueChoice(pOut, "->");
      }

      void AppendFunctionNameChoices(const GraphDocument& document,
                                     std::vector<std::string>* pOut)
      {
         if (pOut == nullptr)
         {
            return;
         }
         const std::vector<Node>& nodes = document.GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            if (nodes[index].type != BlockType::FunctionDef)
            {
               continue;
            }
            const auto nameIterator = nodes[index].properties.find("name");
            if ((nameIterator == nodes[index].properties.end()) ||
                nameIterator->second.empty())
            {
               continue;
            }
            AppendUniqueChoice(pOut, nameIterator->second);
         }
      }
   } // namespace

   PropertyPanel::PropertyPanel(const sf::Font& font)
      : _pFont(&font)
   {
   }

   void PropertyPanel::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      CloseChoicePopup();
      RebuildFields();
   }

   void PropertyPanel::SetTheme(const UiTheme& theme)
   {
      _theme = theme;
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
      CloseChoicePopup();
      _pDocument = pDocument;
      _selectedNodeId = selectedNodeId;
      _activeFieldIndex = -1;
      RebuildFields();
   }

   void PropertyPanel::ReloadFromDocument(void)
   {
      CloseChoicePopup();
      _activeFieldIndex = -1;
      RebuildFields();
   }

   void PropertyPanel::Blur(void)
   {
      CommitActiveField();
      CloseChoicePopup();
      _activeFieldIndex = -1;
   }

   bool PropertyPanel::HasKeyboardFocus(void) const
   {
      return _activeFieldIndex >= 0;
   }

   void PropertyPanel::CloseChoicePopup(void)
   {
      _choicePopupOpen = false;
      _choicePopupFieldIndex = -1;
      _choiceScrollOffset = 0;
      _choiceItems.clear();
      _choicePopupBounds = sf::FloatRect {};
   }

   void PropertyPanel::RebuildChoicePopupBounds(void)
   {
      if ((!_choicePopupOpen) || (_choicePopupFieldIndex < 0) ||
          (static_cast<size_t>(_choicePopupFieldIndex) >= _fields.size()))
      {
         CloseChoicePopup();
         return;
      }
      const Field& field = _fields[static_cast<size_t>(_choicePopupFieldIndex)];
      const float width = field.bounds.size.x;
      const float fullHeight =
         static_cast<float>(_choiceItems.size()) * ChoiceItemHeight;
      const bool needsScroll = (fullHeight > ChoicePopupMaxHeight);
      const float footerHeight = needsScroll ? 16.0f : 0.0f;
      const float height =
         needsScroll ? ChoicePopupMaxHeight : fullHeight;
      const float listHeight = height - footerHeight;
      const auto visibleCount =
         static_cast<int32_t>(listHeight / ChoiceItemHeight);
      const auto itemCount = static_cast<int32_t>(_choiceItems.size());
      const int32_t maxScroll =
         std::max(static_cast<int32_t>(0), itemCount - visibleCount);
      if (_choiceScrollOffset > maxScroll)
      {
         _choiceScrollOffset = maxScroll;
      }
      if (_choiceScrollOffset < 0)
      {
         _choiceScrollOffset = 0;
      }

      _choicePopupBounds =
         sf::FloatRect(sf::Vector2f(field.bounds.position.x,
                                    field.bounds.position.y + field.bounds.size.y + 2.0f),
                       sf::Vector2f(width, height));
      for (size_t index = 0; index < _choiceItems.size(); ++index)
      {
         const auto visualIndex =
            static_cast<int32_t>(index) - _choiceScrollOffset;
         if ((visualIndex < 0) || (visualIndex >= visibleCount))
         {
            _choiceItems[index].bounds = sf::FloatRect {};
            continue;
         }
         const float itemY =
            _choicePopupBounds.position.y +
            (static_cast<float>(visualIndex) * ChoiceItemHeight);
         _choiceItems[index].bounds =
            sf::FloatRect(sf::Vector2f(_choicePopupBounds.position.x, itemY),
                          sf::Vector2f(width, ChoiceItemHeight));
      }
   }

   void PropertyPanel::OpenChoicePopup(int32_t fieldIndex)
   {
      CloseChoicePopup();
      if ((fieldIndex < 0) || (static_cast<size_t>(fieldIndex) >= _fields.size()))
      {
         return;
      }
      const Field& field = _fields[static_cast<size_t>(fieldIndex)];
      if ((field.editKind != FieldEditKind::Choice) || field.choices.empty())
      {
         return;
      }
      _choicePopupOpen = true;
      _choicePopupFieldIndex = fieldIndex;
      _choiceScrollOffset = 0;
      _choiceItems.clear();
      for (size_t index = 0; index < field.choices.size(); ++index)
      {
         ChoiceItem item;
         item.label = field.choices[index];
         _choiceItems.push_back(item);
      }
      RebuildChoicePopupBounds();
   }

   void PropertyPanel::FillChoicesForField(Field* pField, BlockType blockType) const
   {
      if (pField == nullptr)
      {
         return;
      }
      pField->editKind = FieldEditKind::Text;
      pField->choices.clear();

      if (IsTypePropertyKey(pField->key))
      {
         pField->editKind = FieldEditKind::Choice;
         AppendPrimitiveTypeChoices(&pField->choices);
         if (_pDocument != nullptr)
         {
            AppendNamedTypesFromDocument(*_pDocument, &pField->choices);
         }
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
      if (pField->key == "paramCount")
      {
         pField->editKind = FieldEditKind::Choice;
         AppendParamCountChoices(&pField->choices);
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
      if (pField->key == "clangFormat")
      {
         pField->editKind = FieldEditKind::Choice;
         AppendYesNoChoices(&pField->choices);
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
      if ((pField->key == "op") && (blockType == BlockType::CompoundAssign))
      {
         pField->editKind = FieldEditKind::Choice;
         AppendCompoundOpChoices(&pField->choices);
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
      if ((pField->key == "access") &&
          ((blockType == BlockType::FieldLoad) || (blockType == BlockType::FieldStore)))
      {
         pField->editKind = FieldEditKind::Choice;
         AppendAccessChoices(&pField->choices);
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
      if ((pField->key == "function") && (blockType == BlockType::Call))
      {
         pField->editKind = FieldEditKind::Choice;
         if (_pDocument != nullptr)
         {
            AppendFunctionNameChoices(*_pDocument, &pField->choices);
         }
         AppendUniqueChoice(&pField->choices, pField->value);
         return;
      }
   }

   float PropertyPanel::FieldsStartY(void) const
   {
      float cursorY = _bounds.position.y + TitleHeight + TypeLabelHeight;
      if (!_helpLines.empty())
      {
         cursorY += static_cast<float>(_helpLines.size()) * HelpLineHeight;
         cursorY += HelpBottomGap;
      }
      if (!_previewLines.empty())
      {
         cursorY += static_cast<float>(_previewLines.size()) * PreviewLineHeight;
         cursorY += PreviewBottomGap;
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

   void PropertyPanel::RebuildPreviewLines(std::string_view previewText)
   {
      _previewLines.clear();
      if (previewText.empty())
      {
         return;
      }
      _previewLines.push_back("C:");
      std::string body(previewText);
      const float maxWidth = _bounds.size.x - 16.0f;
      if ((_pFont == nullptr) || (maxWidth < 20.0f))
      {
         _previewLines.push_back(body);
         return;
      }

      std::string currentLine;
      for (size_t index = 0; index < body.size(); ++index)
      {
         currentLine.push_back(body[index]);
         sf::Text measure(*_pFont, currentLine, PreviewCharacterSize);
         if (measure.getLocalBounds().size.x > maxWidth)
         {
            if (currentLine.size() <= 1)
            {
               _previewLines.push_back(currentLine);
               currentLine.clear();
               continue;
            }
            char last = currentLine.back();
            currentLine.pop_back();
            if (!currentLine.empty())
            {
               _previewLines.push_back(currentLine);
            }
            currentLine.clear();
            currentLine.push_back(last);
         }
         if (_previewLines.size() >= 4)
         {
            break;
         }
      }
      if ((!currentLine.empty()) && (_previewLines.size() < 4))
      {
         _previewLines.push_back(currentLine);
      }
   }

   void PropertyPanel::RebuildFields(void)
   {
      _fields.clear();
      _helpLines.clear();
      _previewLines.clear();
      if (_pDocument == nullptr)
      {
         return;
      }
      if (_selectedNodeId == 0)
      {
         RebuildHelpLines(
            "Document settings. fileDescription becomes the generated "
            "\\\\brief. clangFormat=1 runs clang-format after Generate when available.");
         float cursorY = FieldsStartY();
         Field descriptionField;
         descriptionField.key = "fileDescription";
         descriptionField.value = _pDocument->GetFileDescription();
         descriptionField.editKind = FieldEditKind::Text;
         descriptionField.bounds =
            sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, cursorY + 16.0f),
                          sf::Vector2f(_bounds.size.x - 16.0f, 22.0f));
         _fields.push_back(descriptionField);
         cursorY += FieldRowHeight;

         Field formatField;
         formatField.key = "clangFormat";
         formatField.value =
            (_pDocument->GetClangFormatOnGenerate() ==
             GraphDocument::ClangFormatOnGenerate::Yes)
               ? "1"
               : "0";
         FillChoicesForField(&formatField, BlockType::Start);
         formatField.bounds =
            sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, cursorY + 16.0f),
                          sf::Vector2f(_bounds.size.x - 16.0f, 22.0f));
         _fields.push_back(formatField);
         return;
      }

      const Node* pNode = _pDocument->FindNode(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }

      RebuildHelpLines(BlockTypeHelpText(pNode->type));
      RebuildPreviewLines(GenerateCSnippet(*_pDocument, _selectedNodeId));

      float cursorY = FieldsStartY();
      for (PropertyMap::const_iterator iterator = pNode->properties.begin();
           iterator != pNode->properties.end();
           ++iterator)
      {
         if (ShouldHidePropertyKey(iterator->first))
         {
            continue;
         }
         Field field;
         field.key = iterator->first;
         field.value = iterator->second;
         FillChoicesForField(&field, pNode->type);
         field.bounds =
            sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, cursorY + 16.0f),
                          sf::Vector2f(_bounds.size.x - 16.0f, 22.0f));
         _fields.push_back(field);
         cursorY += FieldRowHeight;
      }
   }

   void PropertyPanel::CommitActiveField(void)
   {
      if ((_activeFieldIndex < 0) || (_pDocument == nullptr))
      {
         return;
      }
      if (static_cast<size_t>(_activeFieldIndex) >= _fields.size())
      {
         return;
      }
      const Field& field = _fields[static_cast<size_t>(_activeFieldIndex)];

      if (_selectedNodeId == 0)
      {
         if (field.key == "fileDescription")
         {
            if (_pDocument->GetFileDescription() == field.value)
            {
               return;
            }
            if (_pHistory != nullptr)
            {
               _pHistory->PushCheckpoint(*_pDocument);
            }
            _pDocument->SetFileDescription(field.value);
            _pDocument->SetDirty(true);
            return;
         }
         if (field.key == "clangFormat")
         {
            const GraphDocument::ClangFormatOnGenerate nextValue =
               (field.value == "1") ? GraphDocument::ClangFormatOnGenerate::Yes
                                    : GraphDocument::ClangFormatOnGenerate::No;
            if (_pDocument->GetClangFormatOnGenerate() == nextValue)
            {
               return;
            }
            if (_pHistory != nullptr)
            {
               _pHistory->PushCheckpoint(*_pDocument);
            }
            _pDocument->SetClangFormatOnGenerate(nextValue);
            _pDocument->SetDirty(true);
            return;
         }
         return;
      }

      Node* pNode = _pDocument->FindNodeMutable(_selectedNodeId);
      if (pNode == nullptr)
      {
         return;
      }
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
      SyncNodePortTypes(pNode);
      SyncFunctionDefParams(pNode);
      SyncPrintfArgVisibility(pNode, _pDocument);
      SyncCallArgPorts(pNode, _pDocument);
      if (pNode->type == BlockType::FunctionDef)
      {
         SyncAllNodePorts(_pDocument);
      }
      _pDocument->SetDirty(true);
      RebuildPreviewLines(GenerateCSnippet(*_pDocument, _selectedNodeId));
      if ((field.key == "paramCount") || (field.key.find("param") == 0))
      {
         const int32_t keepIndex = _activeFieldIndex;
         RebuildFields();
         if ((keepIndex >= 0) &&
             (static_cast<size_t>(keepIndex) < _fields.size()))
         {
            _activeFieldIndex = keepIndex;
         }
      }
   }

   bool PropertyPanel::HandleChoicePopupClick(sf::Vector2f point)
   {
      if (!_choicePopupOpen)
      {
         return false;
      }
      if (!_choicePopupBounds.contains(point))
      {
         CloseChoicePopup();
         return false;
      }
      for (size_t index = 0; index < _choiceItems.size(); ++index)
      {
         if (!_choiceItems[index].bounds.contains(point))
         {
            continue;
         }
         if ((_choicePopupFieldIndex < 0) ||
             (static_cast<size_t>(_choicePopupFieldIndex) >= _fields.size()))
         {
            CloseChoicePopup();
            return true;
         }
         _activeFieldIndex = _choicePopupFieldIndex;
         _fields[static_cast<size_t>(_choicePopupFieldIndex)].value =
            _choiceItems[index].label;
         CommitActiveField();
         CloseChoicePopup();
         _activeFieldIndex = -1;
         return true;
      }
      return true;
   }

   bool PropertyPanel::HandleClick(sf::Vector2f point)
   {
      if (_choicePopupOpen)
      {
         if (HandleChoicePopupClick(point))
         {
            return true;
         }
      }
      if (!_bounds.contains(point))
      {
         return false;
      }
      CommitActiveField();
      _activeFieldIndex = -1;
      for (size_t index = 0; index < _fields.size(); ++index)
      {
         if (!_fields[index].bounds.contains(point))
         {
            continue;
         }
         _activeFieldIndex = static_cast<int32_t>(index);
         if (_fields[index].editKind == FieldEditKind::Choice)
         {
            OpenChoicePopup(_activeFieldIndex);
         }
         else
         {
            CloseChoicePopup();
         }
         return true;
      }
      CloseChoicePopup();
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
      if (_choicePopupOpen && (keyCode == sf::Keyboard::Key::Escape))
      {
         CloseChoicePopup();
         return true;
      }
      if (_activeFieldIndex < 0)
      {
         return false;
      }
      if (keyCode == sf::Keyboard::Key::Enter)
      {
         CommitActiveField();
         CloseChoicePopup();
         _activeFieldIndex = -1;
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Escape)
      {
         RebuildFields();
         CloseChoicePopup();
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

   bool PropertyPanel::HandleWheel(float delta, sf::Vector2f point)
   {
      if (!_choicePopupOpen)
      {
         return false;
      }
      if (!_choicePopupBounds.contains(point))
      {
         return false;
      }
      if (delta > 0.0f)
      {
         --_choiceScrollOffset;
      }
      else if (delta < 0.0f)
      {
         ++_choiceScrollOffset;
      }
      RebuildChoicePopupBounds();
      return true;
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
      background.setFillColor(_theme.panelBackground);
      pTarget->draw(background);

      sf::Text title(*_pFont, "Properties", 16);
      title.setFillColor(_theme.textPrimary);
      title.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                     _bounds.position.y + 6.0f));
      pTarget->draw(title);

      if (_pDocument == nullptr)
      {
         sf::Text empty(*_pFont, "No document", 13);
         empty.setFillColor(_theme.textMuted);
         empty.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                        _bounds.position.y + TitleHeight));
         pTarget->draw(empty);
         return;
      }

      if (_selectedNodeId == 0)
      {
         sf::Text typeLabel(*_pFont, "Type: Document", 13);
         typeLabel.setFillColor(_theme.textSecondary);
         typeLabel.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                            _bounds.position.y + TitleHeight));
         pTarget->draw(typeLabel);
      }
      else
      {
         const Node* pNode = _pDocument->FindNode(_selectedNodeId);
         if (pNode == nullptr)
         {
            return;
         }

         sf::Text typeLabel(
            *_pFont,
            std::string("Type: ") + std::string(BlockTypeLabel(pNode->type)),
            13);
         typeLabel.setFillColor(_theme.textSecondary);
         typeLabel.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                            _bounds.position.y + TitleHeight));
         pTarget->draw(typeLabel);
      }

      float helpY = _bounds.position.y + TitleHeight + TypeLabelHeight;
      for (size_t lineIndex = 0; lineIndex < _helpLines.size(); ++lineIndex)
      {
         sf::Text helpLine(*_pFont, _helpLines[lineIndex], HelpCharacterSize);
         helpLine.setFillColor(_theme.textHelp);
         helpLine.setPosition(sf::Vector2f(_bounds.position.x + 8.0f, helpY));
         pTarget->draw(helpLine);
         helpY += HelpLineHeight;
      }
      if (!_helpLines.empty())
      {
         helpY += HelpBottomGap;
      }

      for (size_t lineIndex = 0; lineIndex < _previewLines.size(); ++lineIndex)
      {
         sf::Text previewLine(*_pFont, _previewLines[lineIndex], PreviewCharacterSize);
         previewLine.setFillColor(_theme.textPreview);
         previewLine.setPosition(sf::Vector2f(_bounds.position.x + 8.0f, helpY));
         pTarget->draw(previewLine);
         helpY += PreviewLineHeight;
      }

      float labelY = FieldsStartY();
      for (size_t index = 0; index < _fields.size(); ++index)
      {
         const Field& field = _fields[index];
         std::string keyLabel = field.key;
         if (field.editKind == FieldEditKind::Choice)
         {
            keyLabel.append("  ▾");
         }
         sf::Text keyText(*_pFont, keyLabel, 12);
         keyText.setFillColor(_theme.textMuted);
         keyText.setPosition(sf::Vector2f(_bounds.position.x + 8.0f, labelY));
         pTarget->draw(keyText);

         sf::RectangleShape box;
         box.setPosition(field.bounds.position);
         box.setSize(field.bounds.size);
         if (static_cast<int32_t>(index) == _activeFieldIndex)
         {
            box.setFillColor(_theme.inputFillFocused);
         }
         else
         {
            box.setFillColor(_theme.inputFill);
         }
         box.setOutlineColor(_theme.inputOutline);
         box.setOutlineThickness(1.0f);
         pTarget->draw(box);

         sf::Text valueText(*_pFont, field.value, 13);
         valueText.setFillColor(_theme.textInput);
         valueText.setPosition(sf::Vector2f(field.bounds.position.x + 4.0f,
                                            field.bounds.position.y + 2.0f));
         pTarget->draw(valueText);
         labelY += FieldRowHeight;
      }

      if (_choicePopupOpen)
      {
         sf::RectangleShape popupBg;
         popupBg.setPosition(_choicePopupBounds.position);
         popupBg.setSize(_choicePopupBounds.size);
         popupBg.setFillColor(_theme.popupBackground);
         popupBg.setOutlineColor(_theme.popupOutline);
         popupBg.setOutlineThickness(1.0f);
         pTarget->draw(popupBg);

         for (size_t index = 0; index < _choiceItems.size(); ++index)
         {
            const ChoiceItem& item = _choiceItems[index];
            if ((item.bounds.size.x <= 0.0f) || (item.bounds.size.y <= 0.0f))
            {
               continue;
            }
            sf::Text itemText(*_pFont, item.label, 12);
            itemText.setFillColor(_theme.textSecondary);
            itemText.setPosition(sf::Vector2f(item.bounds.position.x + 6.0f,
                                              item.bounds.position.y + 3.0f));
            pTarget->draw(itemText);
         }

         const auto visibleCount =
            static_cast<int32_t>((_choicePopupBounds.size.y - 16.0f) / ChoiceItemHeight);
         const auto itemCount = static_cast<int32_t>(_choiceItems.size());
         if ((itemCount > visibleCount) && (visibleCount > 0))
         {
            sf::Text scrollHint(*_pFont, "scroll for more", 10);
            scrollHint.setFillColor(_theme.textMuted);
            scrollHint.setPosition(sf::Vector2f(
               _choicePopupBounds.position.x + 6.0f,
               _choicePopupBounds.position.y + _choicePopupBounds.size.y - 14.0f));
            pTarget->draw(scrollHint);
         }
      }
   }
} // namespace Cgen
