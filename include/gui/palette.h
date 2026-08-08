/*!
 *\file palette.h
 *\brief Left-side block palette.
 */
#ifndef PALETTE_H
#define PALETTE_H

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
       *\brief Hit-tests a click and returns a block type to place.
       *
       *\param[in] point Mouse position.
       *\param[out] pOutType Selected type on hit.
       *\return true if an entry was clicked.
       */
      bool HitTest(sf::Vector2f point, BlockType* pOutType) const;

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

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<Entry> _entries;
   };
} // namespace Cgen

#endif // PALETTE_H
