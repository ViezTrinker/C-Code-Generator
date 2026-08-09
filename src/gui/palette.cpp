/*!
 *\file palette.cpp
 *\brief Block palette with collapsible category groups.
 */
#include "gui/palette.h"

#include <string>

namespace Cgen
{
   namespace
   {
      constexpr float TitleHeight = 28.0f;
      constexpr float RowHeight = 24.0f;
      constexpr float RowGap = 2.0f;
      constexpr float RowStride = RowHeight + RowGap;
      constexpr float BlockIndent = 14.0f;

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
         BlockType::Free
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
         {"Structs", StructTypes, sizeof(StructTypes) / sizeof(StructTypes[0]),
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

   sf::FloatRect Palette::ListBounds(void) const
   {
      float height = _bounds.size.y - TitleHeight;
      if (height < RowHeight)
      {
         height = RowHeight;
      }
      return sf::FloatRect(
         sf::Vector2f(_bounds.position.x, _bounds.position.y + TitleHeight),
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

   PaletteClickResult Palette::HandleClick(sf::Vector2f point, BlockType* pOutType)
   {
      if (!ListBounds().contains(point))
      {
         return PaletteClickResult::Ignored;
      }

      for (size_t index = 0; index < _visibleRows.size(); ++index)
      {
         const Row& row = _visibleRows[index];
         if (!row.bounds.contains(point))
         {
            continue;
         }

         if (row.kind == RowKind::GroupHeader)
         {
            const size_t groupIndex = static_cast<size_t>(row.groupIndex);
            if (groupIndex < _groupExpanded.size())
            {
               _groupExpanded[groupIndex] = !_groupExpanded[groupIndex];
               RebuildVisibleRows();
            }
            return PaletteClickResult::Consumed;
         }

         if (pOutType != nullptr)
         {
            *pOutType = row.type;
         }
         return PaletteClickResult::PlaceBlock;
      }

      return PaletteClickResult::Consumed;
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
         if (row.kind == RowKind::GroupHeader)
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
         if (row.kind == RowKind::GroupHeader)
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
} // namespace Cgen
