/*!
 *\file palette.cpp
 *\brief Block palette with collapsible category groups.
 */
#include "gui/palette.h"

#include "gui/hover_tooltip.h"
#include "gui/hover_tooltip_text.h"

#include <cctype>
#include <string>
#include <string_view>

namespace Cgen
{
   namespace
   {
      constexpr float TitleHeight = 28.0f;
      constexpr float FilterHeight = 28.0f;
      constexpr float RowHeight = 24.0f;
      constexpr float RowGap = 2.0f;
      constexpr float RowStride = RowHeight + RowGap;
      constexpr float BlockIndent = 14.0f;

      bool ContainsIgnoreCase(std::string_view haystack, std::string_view needle)
      {
         if (needle.empty())
         {
            return true;
         }
         if (haystack.size() < needle.size())
         {
            return false;
         }
         for (size_t start = 0; start + needle.size() <= haystack.size(); ++start)
         {
            bool match = true;
            for (size_t index = 0; index < needle.size(); ++index)
            {
               const auto left =
                  static_cast<unsigned char>(haystack[start + index]);
               const auto right = static_cast<unsigned char>(needle[index]);
               if (std::tolower(left) != std::tolower(right))
               {
                  match = false;
                  break;
               }
            }
            if (match)
            {
               return true;
            }
         }
         return false;
      }

      constexpr BlockType ControlFlowTypes[] = {
         BlockType::End,
         BlockType::If,
         BlockType::ElseIf,
         BlockType::Switch,
         BlockType::Case,
         BlockType::While,
         BlockType::For,
         BlockType::Break,
         BlockType::Continue
      };

      constexpr BlockType DataTypes[] = {
         BlockType::Literal,
         BlockType::VariableDecl,
         BlockType::GlobalDecl,
         BlockType::VariableRef,
         BlockType::Assign,
         BlockType::CompoundAssign,
         BlockType::Inc,
         BlockType::Dec,
         BlockType::Cast
      };

      constexpr BlockType ArithmeticTypes[] = {
         BlockType::Add,
         BlockType::Sub,
         BlockType::Mul,
         BlockType::Div,
         BlockType::Mod,
         BlockType::Neg
      };

      constexpr BlockType LogicTypes[] = {
         BlockType::Equal,
         BlockType::NotEqual,
         BlockType::Less,
         BlockType::LessEqual,
         BlockType::Greater,
         BlockType::GreaterEqual,
         BlockType::And,
         BlockType::Or,
         BlockType::Not
      };

      constexpr BlockType ConsoleIoTypes[] = {
         BlockType::Printf,
         BlockType::WaitEnter,
         BlockType::ScanfInt,
         BlockType::ScanfChar,
         BlockType::ScanfFloat,
         BlockType::ScanfLine
      };

      constexpr BlockType ArrayStringTypes[] = {
         BlockType::ArrayDecl,
         BlockType::IndexAssign,
         BlockType::IndexLoad,
         BlockType::StrLen,
         BlockType::StrCpy,
         BlockType::StrNCpy,
         BlockType::StrCmp,
         BlockType::ShuffleArray
      };

      constexpr BlockType FileTypes[] = {
         BlockType::FileOpen,
         BlockType::FileRead,
         BlockType::FileWrite,
         BlockType::FileClose,
         BlockType::FilePrintf,
         BlockType::FileGets
      };

      constexpr BlockType MemoryTypes[] = {
         BlockType::Malloc,
         BlockType::Free,
         BlockType::AddressOf,
         BlockType::DerefLoad,
         BlockType::DerefStore
      };

      constexpr BlockType TimeRandomTypes[] = {
         BlockType::TimeNow,
         BlockType::LocalTime,
         BlockType::Sleep,
         BlockType::Random,
         BlockType::RandomChar
      };

      constexpr BlockType StructTypes[] = {
         BlockType::StructDecl,
         BlockType::EnumDecl,
         BlockType::TypedefDecl,
         BlockType::StructLiteral,
         BlockType::FieldLoad,
         BlockType::FieldStore
      };

      constexpr BlockType FunctionTypes[] = {
         BlockType::FunctionDef,
         BlockType::Return,
         BlockType::Call
      };

      constexpr BlockType TeachingTypes[] = {
         BlockType::Assert,
         BlockType::Comment
      };

      struct GroupDef
      {
         const char* pTitle;
         const BlockType* pTypes;
         size_t typeCount;
         bool expandByDefault;
      };

      constexpr GroupDef PaletteGroups[] = {
         {"Control Flow", ControlFlowTypes,
          sizeof(ControlFlowTypes) / sizeof(ControlFlowTypes[0]), true},
         {"Data", DataTypes, sizeof(DataTypes) / sizeof(DataTypes[0]), true},
         {"Arithmetic", ArithmeticTypes,
          sizeof(ArithmeticTypes) / sizeof(ArithmeticTypes[0]), false},
         {"Compare / Logic", LogicTypes,
          sizeof(LogicTypes) / sizeof(LogicTypes[0]), false},
         {"Console I/O", ConsoleIoTypes,
          sizeof(ConsoleIoTypes) / sizeof(ConsoleIoTypes[0]), false},
         {"Arrays / Strings", ArrayStringTypes,
          sizeof(ArrayStringTypes) / sizeof(ArrayStringTypes[0]), false},
         {"Files", FileTypes, sizeof(FileTypes) / sizeof(FileTypes[0]), false},
         {"Memory", MemoryTypes, sizeof(MemoryTypes) / sizeof(MemoryTypes[0]),
          false},
         {"Time / Random", TimeRandomTypes,
          sizeof(TimeRandomTypes) / sizeof(TimeRandomTypes[0]), false},
         {"Structs / Types", StructTypes, sizeof(StructTypes) / sizeof(StructTypes[0]),
          false},
         {"Functions", FunctionTypes,
          sizeof(FunctionTypes) / sizeof(FunctionTypes[0]), false},
         {"Teaching", TeachingTypes,
          sizeof(TeachingTypes) / sizeof(TeachingTypes[0]), false}
      };

      constexpr size_t PaletteGroupCount =
         sizeof(PaletteGroups) / sizeof(PaletteGroups[0]);
   } // namespace

   Palette::Palette(const sf::Font& font)
      : _pFont(&font)
   {
      _groupExpanded.resize(PaletteGroupCount, false);
      for (size_t index = 0; index < PaletteGroupCount; ++index)
      {
         _groupExpanded[index] = PaletteGroups[index].expandByDefault;
      }
      RebuildVisibleRows();
   }

   sf::FloatRect Palette::FilterBounds(void) const
   {
      return sf::FloatRect(
         sf::Vector2f(_bounds.position.x + 6.0f,
                      _bounds.position.y + TitleHeight + 2.0f),
         sf::Vector2f(_bounds.size.x - 12.0f, FilterHeight - 4.0f));
   }

   sf::FloatRect Palette::ListBounds(void) const
   {
      const float topOffset = TitleHeight + FilterHeight;
      float height = _bounds.size.y - topOffset;
      if (height < RowHeight)
      {
         height = RowHeight;
      }
      return sf::FloatRect(
         sf::Vector2f(_bounds.position.x, _bounds.position.y + topOffset),
         sf::Vector2f(_bounds.size.x, height));
   }

   uint32_t Palette::VisibleRowCapacity(void) const
   {
      const auto capacity =
         static_cast<uint32_t>(ListBounds().size.y / RowStride);
      if (capacity < 1)
      {
         return 1;
      }
      return capacity;
   }

   uint32_t Palette::MaxScrollRows(void) const
   {
      const auto rowCount = static_cast<uint32_t>(_visibleRows.size());
      const uint32_t capacity = VisibleRowCapacity();
      if (rowCount <= capacity)
      {
         return 0;
      }
      return rowCount - capacity;
   }

   void Palette::ClampScroll(void)
   {
      const uint32_t maxScroll = MaxScrollRows();
      if (_scrollRows > maxScroll)
      {
         _scrollRows = maxScroll;
      }
   }

   void Palette::RebuildVisibleRows(void)
   {
      _visibleRows.clear();
      if (!_filterText.empty())
      {
         for (size_t groupIndex = 0; groupIndex < PaletteGroupCount; ++groupIndex)
         {
            const GroupDef& group = PaletteGroups[groupIndex];
            for (size_t typeIndex = 0; typeIndex < group.typeCount; ++typeIndex)
            {
               const BlockType blockType = group.pTypes[typeIndex];
               if (!ContainsIgnoreCase(BlockTypeLabel(blockType), _filterText))
               {
                  continue;
               }
               Row blockRow;
               blockRow.kind = RowKind::Block;
               blockRow.groupIndex = static_cast<uint32_t>(groupIndex);
               blockRow.type = blockType;
               _visibleRows.push_back(blockRow);
            }
         }
         ClampScroll();
         RebuildRowBounds();
         return;
      }

      for (size_t groupIndex = 0; groupIndex < PaletteGroupCount; ++groupIndex)
      {
         Row header;
         header.kind = RowKind::GroupHeader;
         header.groupIndex = static_cast<uint32_t>(groupIndex);
         _visibleRows.push_back(header);

         if (!_groupExpanded[groupIndex])
         {
            continue;
         }

         const GroupDef& group = PaletteGroups[groupIndex];
         for (size_t typeIndex = 0; typeIndex < group.typeCount; ++typeIndex)
         {
            Row blockRow;
            blockRow.kind = RowKind::Block;
            blockRow.groupIndex = static_cast<uint32_t>(groupIndex);
            blockRow.type = group.pTypes[typeIndex];
            _visibleRows.push_back(blockRow);
         }
      }
      ClampScroll();
      RebuildRowBounds();
   }

   void Palette::RebuildRowBounds(void)
   {
      const sf::FloatRect listBounds = ListBounds();
      float cursorY =
         listBounds.position.y - (static_cast<float>(_scrollRows) * RowStride);
      for (size_t index = 0; index < _visibleRows.size(); ++index)
      {
         float left = _bounds.position.x + 6.0f;
         float width = _bounds.size.x - 12.0f;
         if (_visibleRows[index].kind == RowKind::Block)
         {
            left += BlockIndent;
            width -= BlockIndent;
         }
         _visibleRows[index].bounds = sf::FloatRect(
            sf::Vector2f(left, cursorY),
            sf::Vector2f(width, RowHeight));
         cursorY += RowStride;
      }
   }

   void Palette::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      ClampScroll();
      RebuildRowBounds();
   }

   bool Palette::Contains(sf::Vector2f point) const
   {
      return _bounds.contains(point);
   }

   void Palette::ClearHover(void)
   {
      _hasHover = false;
      _hoverKind = RowKind::Block;
      _hoverGroupIndex = 0;
      _hoverType = BlockType::End;
   }

   void Palette::ClearPressed(void)
   {
      _hasPressed = false;
      _pressedKind = RowKind::Block;
      _pressedGroupIndex = 0;
      _pressedType = BlockType::End;
   }

   bool Palette::FindRowAtPoint(sf::Vector2f point, Row* pOutRow) const
   {
      if (pOutRow == nullptr)
      {
         return false;
      }
      if (!ListBounds().contains(point))
      {
         return false;
      }
      for (size_t index = 0; index < _visibleRows.size(); ++index)
      {
         if (!_visibleRows[index].bounds.contains(point))
         {
            continue;
         }
         *pOutRow = _visibleRows[index];
         return true;
      }
      return false;
   }

   bool Palette::RowMatchesHighlight(const Row& row,
                                     RowKind kind,
                                     uint32_t groupIndex,
                                     BlockType type) const
   {
      if (row.kind != kind)
      {
         return false;
      }
      if (kind == RowKind::GroupHeader)
      {
         return (row.groupIndex == groupIndex);
      }
      return (row.type == type);
   }

   PaletteClickResult Palette::HandleClick(sf::Vector2f point, BlockType* pOutType)
   {
      if (!_bounds.contains(point))
      {
         return PaletteClickResult::Ignored;
      }

      if (FilterBounds().contains(point))
      {
         _filterFocused = true;
         ClearPressed();
         return PaletteClickResult::Consumed;
      }
      _filterFocused = false;

      Row hitRow;
      if (!FindRowAtPoint(point, &hitRow))
      {
         ClearPressed();
         return PaletteClickResult::Consumed;
      }

      _hasPressed = true;
      _pressedKind = hitRow.kind;
      _pressedGroupIndex = hitRow.groupIndex;
      _pressedType = hitRow.type;
      _hasHover = true;
      _hoverKind = hitRow.kind;
      _hoverGroupIndex = hitRow.groupIndex;
      _hoverType = hitRow.type;

      if (hitRow.kind == RowKind::GroupHeader)
      {
         const size_t groupIndex = static_cast<size_t>(hitRow.groupIndex);
         if (groupIndex < _groupExpanded.size())
         {
            _groupExpanded[groupIndex] = !_groupExpanded[groupIndex];
            RebuildVisibleRows();
         }
         _hasSelectedGroup = true;
         _selectedGroupIndex = hitRow.groupIndex;
         return PaletteClickResult::Consumed;
      }

      _hasSelectedBlock = true;
      _selectedBlockType = hitRow.type;
      if (pOutType != nullptr)
      {
         *pOutType = hitRow.type;
      }
      return PaletteClickResult::PlaceBlock;
   }

   void Palette::HandleMouseMove(sf::Vector2f point)
   {
      _hoverPoint = point;
      Row hitRow;
      if (!FindRowAtPoint(point, &hitRow))
      {
         ClearHover();
         return;
      }
      _hasHover = true;
      _hoverKind = hitRow.kind;
      _hoverGroupIndex = hitRow.groupIndex;
      _hoverType = hitRow.type;
   }

   void Palette::HandleMouseRelease(void)
   {
      ClearPressed();
   }

   bool Palette::HandleTextEntered(uint32_t unicode)
   {
      if (!_filterFocused)
      {
         return false;
      }
      if ((unicode == 8) || (unicode == 127))
      {
         if (!_filterText.empty())
         {
            _filterText.pop_back();
            _scrollRows = 0;
            RebuildVisibleRows();
         }
         return true;
      }
      if ((unicode >= 32) && (unicode < 127))
      {
         _filterText.push_back(static_cast<char>(unicode));
         _scrollRows = 0;
         RebuildVisibleRows();
         return true;
      }
      return false;
   }

   bool Palette::HandleKey(sf::Keyboard::Key keyCode)
   {
      if (!_filterFocused)
      {
         return false;
      }
      if (keyCode == sf::Keyboard::Key::Escape)
      {
         if (!_filterText.empty())
         {
            _filterText.clear();
            _scrollRows = 0;
            RebuildVisibleRows();
         }
         else
         {
            _filterFocused = false;
         }
         return true;
      }
      if ((keyCode == sf::Keyboard::Key::Backspace) ||
          (keyCode == sf::Keyboard::Key::Delete))
      {
         return true;
      }
      return false;
   }

   void Palette::BlurFilter(void)
   {
      _filterFocused = false;
   }

   bool Palette::IsFilterFocused(void) const
   {
      return _filterFocused;
   }
   bool Palette::HandleWheel(float delta, sf::Vector2f point)
   {
      if (!_bounds.contains(point))
      {
         return false;
      }

      const uint32_t maxScroll = MaxScrollRows();
      if (maxScroll == 0)
      {
         return true;
      }

      if (delta > 0.0f)
      {
         if (_scrollRows > 0)
         {
            --_scrollRows;
         }
      }
      else if (delta < 0.0f)
      {
         if (_scrollRows < maxScroll)
         {
            ++_scrollRows;
         }
      }

      RebuildRowBounds();
      return true;
   }

   void Palette::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }
      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(35, 38, 44));
      background.setOutlineColor(sf::Color(70, 76, 90));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      sf::Text title(*_pFont, "Blocks", 16);
      title.setFillColor(sf::Color::White);
      title.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                     _bounds.position.y + 6.0f));
      pTarget->draw(title);

      const sf::FloatRect filterBounds = FilterBounds();
      sf::RectangleShape filterBox;
      filterBox.setPosition(filterBounds.position);
      filterBox.setSize(filterBounds.size);
      if (_filterFocused)
      {
         filterBox.setFillColor(sf::Color(50, 60, 80));
      }
      else
      {
         filterBox.setFillColor(sf::Color(40, 44, 52));
      }
      filterBox.setOutlineColor(sf::Color(100, 120, 150));
      filterBox.setOutlineThickness(1.0f);
      pTarget->draw(filterBox);

      std::string filterDisplay = _filterText;
      if (filterDisplay.empty() && (!_filterFocused))
      {
         filterDisplay = "Filter...";
      }
      else if (_filterFocused)
      {
         filterDisplay.push_back('_');
      }
      sf::Text filterText(*_pFont, filterDisplay, 12);
      if (_filterText.empty() && (!_filterFocused))
      {
         filterText.setFillColor(sf::Color(140, 140, 140));
      }
      else
      {
         filterText.setFillColor(sf::Color(230, 230, 200));
      }
      filterText.setPosition(sf::Vector2f(filterBounds.position.x + 6.0f,
                                          filterBounds.position.y + 4.0f));
      pTarget->draw(filterText);

      const sf::FloatRect listBounds = ListBounds();
      const sf::Vector2u targetSize = pTarget->getSize();
      if ((targetSize.x == 0) || (targetSize.y == 0))
      {
         return;
      }

      const sf::View previousView = pTarget->getView();
      sf::View clipView;
      clipView.setSize(listBounds.size);
      clipView.setCenter(sf::Vector2f(
         listBounds.position.x + (listBounds.size.x * 0.5f),
         listBounds.position.y + (listBounds.size.y * 0.5f)));
      clipView.setViewport(sf::FloatRect(
         sf::Vector2f(listBounds.position.x / static_cast<float>(targetSize.x),
                      listBounds.position.y / static_cast<float>(targetSize.y)),
         sf::Vector2f(listBounds.size.x / static_cast<float>(targetSize.x),
                      listBounds.size.y / static_cast<float>(targetSize.y))));
      pTarget->setView(clipView);

      const float listBottom = listBounds.position.y + listBounds.size.y;
      for (size_t index = 0; index < _visibleRows.size(); ++index)
      {
         const Row& row = _visibleRows[index];
         const float entryBottom = row.bounds.position.y + row.bounds.size.y;
         if (entryBottom < listBounds.position.y)
         {
            continue;
         }
         if (row.bounds.position.y > listBottom)
         {
            continue;
         }

         sf::RectangleShape rowBackground;
         rowBackground.setPosition(row.bounds.position);
         rowBackground.setSize(row.bounds.size);

         const bool isPressed =
            _hasPressed &&
            RowMatchesHighlight(row, _pressedKind, _pressedGroupIndex, _pressedType);
         const bool isSelectedBlock =
            _hasSelectedBlock && (row.kind == RowKind::Block) &&
            (row.type == _selectedBlockType);
         const bool isSelectedGroup =
            _hasSelectedGroup && (row.kind == RowKind::GroupHeader) &&
            (row.groupIndex == _selectedGroupIndex);
         const bool isHovered =
            _hasHover &&
            RowMatchesHighlight(row, _hoverKind, _hoverGroupIndex, _hoverType);

         if (isPressed)
         {
            rowBackground.setFillColor(sf::Color(70, 115, 175));
            rowBackground.setOutlineColor(sf::Color(150, 195, 245));
            rowBackground.setOutlineThickness(1.0f);
         }
         else if (isSelectedBlock || isSelectedGroup)
         {
            rowBackground.setFillColor(sf::Color(55, 95, 145));
            rowBackground.setOutlineColor(sf::Color(130, 175, 230));
            rowBackground.setOutlineThickness(1.0f);
         }
         else if (isHovered)
         {
            if (row.kind == RowKind::GroupHeader)
            {
               rowBackground.setFillColor(sf::Color(72, 84, 105));
            }
            else
            {
               rowBackground.setFillColor(sf::Color(68, 82, 105));
            }
            rowBackground.setOutlineColor(sf::Color(120, 145, 180));
            rowBackground.setOutlineThickness(1.0f);
         }
         else if (row.kind == RowKind::GroupHeader)
         {
            rowBackground.setFillColor(sf::Color(58, 64, 78));
         }
         else
         {
            rowBackground.setFillColor(sf::Color(50, 54, 62));
         }
         pTarget->draw(rowBackground);

         std::string labelText;
         if (row.kind == RowKind::GroupHeader)
         {
            const size_t groupIndex = static_cast<size_t>(row.groupIndex);
            const bool expanded =
               (groupIndex < _groupExpanded.size()) && _groupExpanded[groupIndex];
            if (expanded)
            {
               labelText = "v  ";
            }
            else
            {
               labelText = ">  ";
            }
            labelText.append(PaletteGroups[groupIndex].pTitle);
         }
         else
         {
            labelText = std::string(BlockTypeLabel(row.type));
         }

         sf::Text label(*_pFont, labelText, 13);
         if (isPressed || isSelectedBlock || isSelectedGroup)
         {
            label.setFillColor(sf::Color(245, 250, 255));
         }
         else if (row.kind == RowKind::GroupHeader)
         {
            label.setFillColor(sf::Color(220, 230, 255));
         }
         else
         {
            label.setFillColor(sf::Color(230, 230, 230));
         }
         label.setPosition(sf::Vector2f(row.bounds.position.x + 6.0f,
                                        row.bounds.position.y + 2.0f));
         pTarget->draw(label);
      }

      pTarget->setView(previousView);
   }

   void Palette::DrawHoverTip(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr) || (!_hasHover))
      {
         return;
      }

      const PaletteRowTipKind tipKind = (_hoverKind == RowKind::GroupHeader)
                                           ? PaletteRowTipKind::GroupHeader
                                           : PaletteRowTipKind::Block;
      const std::string_view tipText =
         PaletteRowHoverTipText(tipKind, _hoverType);

      constexpr float TooltipMaxWidth = 320.0f;
      DrawHoverTooltip(pTarget, *_pFont, _hoverPoint, tipText, TooltipMaxWidth);
   }
} // namespace Cgen
