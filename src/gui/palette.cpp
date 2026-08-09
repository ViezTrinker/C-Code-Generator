/*!
 *\file palette.cpp
 *\brief Block palette implementation.
 */
#include "gui/palette.h"

#include <string>

namespace Cgen
{
   namespace
   {
      constexpr BlockType PaletteTypes[] = {
         BlockType::End,
         BlockType::If,
         BlockType::ElseIf,
         BlockType::Switch,
         BlockType::Case,
         BlockType::While,
         BlockType::For,
         BlockType::Break,
         BlockType::Continue,
         BlockType::Literal,
         BlockType::VariableDecl,
         BlockType::GlobalDecl,
         BlockType::VariableRef,
         BlockType::Assign,
         BlockType::CompoundAssign,
         BlockType::Inc,
         BlockType::Dec,
         BlockType::Add,
         BlockType::Sub,
         BlockType::Mul,
         BlockType::Div,
         BlockType::Mod,
         BlockType::Neg,
         BlockType::Cast,
         BlockType::Equal,
         BlockType::NotEqual,
         BlockType::Less,
         BlockType::LessEqual,
         BlockType::Greater,
         BlockType::GreaterEqual,
         BlockType::And,
         BlockType::Or,
         BlockType::Not,
         BlockType::Printf,
         BlockType::WaitEnter,
         BlockType::ScanfInt,
         BlockType::ScanfChar,
         BlockType::ScanfFloat,
         BlockType::ScanfLine,
         BlockType::ArrayDecl,
         BlockType::IndexAssign,
         BlockType::IndexLoad,
         BlockType::StrLen,
         BlockType::StrCpy,
         BlockType::StrNCpy,
         BlockType::StrCmp,
         BlockType::FileOpen,
         BlockType::FileRead,
         BlockType::FileWrite,
         BlockType::FileClose,
         BlockType::Assert,
         BlockType::Comment,
         BlockType::StructDecl,
         BlockType::FieldLoad,
         BlockType::FieldStore,
         BlockType::RandomChar,
         BlockType::ShuffleArray,
         BlockType::Malloc,
         BlockType::Free,
         BlockType::TimeNow,
         BlockType::LocalTime,
         BlockType::Sleep,
         BlockType::Random,
         BlockType::FunctionDef,
         BlockType::Return,
         BlockType::Call
      };

      constexpr size_t PaletteTypeCount = sizeof(PaletteTypes) / sizeof(PaletteTypes[0]);
      constexpr float TitleHeight = 28.0f;
      constexpr float RowHeight = 24.0f;
      constexpr float RowGap = 2.0f;
      constexpr float RowStride = RowHeight + RowGap;
   } // namespace

   Palette::Palette(const sf::Font& font)
      : _pFont(&font)
   {
      for (size_t index = 0; index < PaletteTypeCount; ++index)
      {
         Entry entry;
         entry.type = PaletteTypes[index];
         _entries.push_back(entry);
      }
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
      const auto entryCount = static_cast<uint32_t>(_entries.size());
      const uint32_t capacity = VisibleRowCapacity();
      if (entryCount <= capacity)
      {
         return 0;
      }
      return entryCount - capacity;
   }

   void Palette::ClampScroll(void)
   {
      const uint32_t maxScroll = MaxScrollRows();
      if (_scrollRows > maxScroll)
      {
         _scrollRows = maxScroll;
      }
   }

   void Palette::RebuildEntryBounds(void)
   {
      const sf::FloatRect listBounds = ListBounds();
      float cursorY =
         listBounds.position.y - (static_cast<float>(_scrollRows) * RowStride);
      for (size_t index = 0; index < _entries.size(); ++index)
      {
         _entries[index].bounds = sf::FloatRect(
            sf::Vector2f(_bounds.position.x + 6.0f, cursorY),
            sf::Vector2f(_bounds.size.x - 12.0f, RowHeight));
         cursorY += RowStride;
      }
   }

   void Palette::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      ClampScroll();
      RebuildEntryBounds();
   }

   bool Palette::Contains(sf::Vector2f point) const
   {
      return _bounds.contains(point);
   }

   bool Palette::HitTest(sf::Vector2f point, BlockType* pOutType) const
   {
      if (pOutType == nullptr)
      {
         return false;
      }
      if (!ListBounds().contains(point))
      {
         return false;
      }
      for (size_t index = 0; index < _entries.size(); ++index)
      {
         if (_entries[index].bounds.contains(point))
         {
            *pOutType = _entries[index].type;
            return true;
         }
      }
      return false;
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

      RebuildEntryBounds();
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

      for (size_t index = 0; index < _entries.size(); ++index)
      {
         const Entry& entry = _entries[index];
         const float entryBottom = entry.bounds.position.y + entry.bounds.size.y;
         const float listBottom = listBounds.position.y + listBounds.size.y;
         if (entryBottom < listBounds.position.y)
         {
            continue;
         }
         if (entry.bounds.position.y > listBottom)
         {
            continue;
         }

         sf::RectangleShape row;
         row.setPosition(entry.bounds.position);
         row.setSize(entry.bounds.size);
         row.setFillColor(sf::Color(50, 54, 62));
         pTarget->draw(row);

         sf::Text label(*_pFont, std::string(BlockTypeLabel(entry.type)), 13);
         label.setFillColor(sf::Color(230, 230, 230));
         label.setPosition(sf::Vector2f(entry.bounds.position.x + 6.0f,
                                        entry.bounds.position.y + 2.0f));
         pTarget->draw(label);
      }

      pTarget->setView(previousView);
   }
} // namespace Cgen
