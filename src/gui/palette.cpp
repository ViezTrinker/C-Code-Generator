/*!
 *\file palette.cpp
 *\brief Block palette implementation.
 */
#include "gui/palette.h"

namespace Cgen
{
   namespace
   {
      constexpr BlockType PaletteTypes[] = {
         BlockType::End,
         BlockType::If,
         BlockType::While,
         BlockType::For,
         BlockType::Literal,
         BlockType::VariableDecl,
         BlockType::GlobalDecl,
         BlockType::VariableRef,
         BlockType::Assign,
         BlockType::Add,
         BlockType::Sub,
         BlockType::Mul,
         BlockType::Div,
         BlockType::Equal,
         BlockType::Less,
         BlockType::Greater,
         BlockType::Printf,
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

   void Palette::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      constexpr float RowHeight = 24.0f;
      float cursorY = bounds.position.y + 28.0f;
      for (size_t index = 0; index < _entries.size(); ++index)
      {
         _entries[index].bounds =
            sf::FloatRect(sf::Vector2f(bounds.position.x + 6.0f, cursorY),
                          sf::Vector2f(bounds.size.x - 12.0f, RowHeight));
         cursorY += RowHeight + 2.0f;
      }
   }

   bool Palette::HitTest(sf::Vector2f point, BlockType* pOutType) const
   {
      if (pOutType == nullptr)
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
      pTarget->draw(background);

      sf::Text title(*_pFont, "Blocks", 16);
      title.setFillColor(sf::Color::White);
      title.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                     _bounds.position.y + 6.0f));
      pTarget->draw(title);

      for (size_t index = 0; index < _entries.size(); ++index)
      {
         const Entry& entry = _entries[index];
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
   }
} // namespace Cgen
