/*!
 *\file palette.h
 *\brief Left-side block palette with collapsible groups.
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
    *\brief Result of a palette mouse click.
    */
   enum class PaletteClickResult: int8_t
   {
      Ignored = 0,
      Consumed = 1,
      PlaceBlock = 2
   };

   /*!
    *\brief Clickable grouped list of placeable blocks.
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
       *\brief Handles a click: toggle a group or select a block to place.
       *
       *\param[in] point Mouse position.
       *\param[out] pOutType Block type when result is PlaceBlock.
       *\return Click result.
       */
      PaletteClickResult HandleClick(sf::Vector2f point, BlockType* pOutType);

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
      enum class RowKind: uint8_t
      {
         GroupHeader = 0,
         Block = 1
      };

      struct Row
      {
         RowKind kind = RowKind::Block;
         uint32_t groupIndex = 0;
         BlockType type = BlockType::End;
         sf::FloatRect bounds {};
      };

      void RebuildVisibleRows(void);
      void RebuildRowBounds(void);
      void ClampScroll(void);
      uint32_t VisibleRowCapacity(void) const;
      uint32_t MaxScrollRows(void) const;
      sf::FloatRect ListBounds(void) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<bool> _groupExpanded;
      std::vector<Row> _visibleRows;
      uint32_t _scrollRows = 0;
   };
} // namespace Cgen

#endif // PALETTE_H
