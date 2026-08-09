/*!
 *\file palette.h
 *\brief Left-side block palette.
 */
#ifndef PALETTE_H
#define PALETTE_H

#include <cstdint>
#include <vector>

#include <SFML/Graphics.hpp>

#include "model/block_type.h"

namespace Cgen
{
   /*!
    *\brief Clickable list of placeable blocks.
    */
   class Palette
   {
   public:
      /*!
       *\brief Constructs the palette.
       *
       *\param[in] font Font for labels.
       */
      explicit Palette(const sf::Font& font);

      /*!
       *\brief Sets palette bounds.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Returns true if the point lies inside the palette.
       *
       *\param[in] point Mouse position.
       */
      bool Contains(sf::Vector2f point) const;

      /*!
       *\brief Hit-tests a click and returns a block type to place.
       *
       *\param[in] point Mouse position.
       *\param[out] pOutType Selected type on hit.
       *\return true if an entry was clicked.
       */
      bool HitTest(sf::Vector2f point, BlockType* pOutType) const;

      /*!
       *\brief Scrolls the block list when the wheel is used over this palette.
       *
       *\param[in] delta Wheel delta.
       *\param[in] point Mouse position.
       *\return true if handled.
       */
      bool HandleWheel(float delta, sf::Vector2f point);

      /*!
       *\brief Draws the palette.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      struct Entry
      {
         BlockType type = BlockType::Start;
         sf::FloatRect bounds {};
      };

      void RebuildEntryBounds(void);
      void ClampScroll(void);
      uint32_t VisibleRowCapacity(void) const;
      uint32_t MaxScrollRows(void) const;
      sf::FloatRect ListBounds(void) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<Entry> _entries;
      uint32_t _scrollRows = 0;
   };
} // namespace Cgen

#endif // PALETTE_H
