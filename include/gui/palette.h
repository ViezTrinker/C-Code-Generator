/*!
 *\file palette.h
 *\brief Left-side block palette with collapsible groups and filter.
 */
#ifndef PALETTE_H
#define PALETTE_H

#include <cstdint>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "gui/ui_theme.h"
#include "model/block_type.h"

namespace Cgen
{
   /*!
    *\brief Result of a palette mouse press.
    */
   enum class PaletteClickResult: int8_t
   {
      Ignored = 0,
      Consumed = 1,
      BeginDrag = 2
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
       *\brief Applies a UI color theme.
       *
       *\param[in] theme Theme palette.
       */
      void SetTheme(const UiTheme& theme);

      /*!
       *\brief Returns true if the point lies inside the palette.
       *
       *\param[in] point Mouse position.
       */
      bool Contains(sf::Vector2f point) const;

      /*!
       *\brief Handles a press: filter focus, toggle a group, or begin a block drag.
       *
       *\param[in] point Mouse position.
       *\param[out] pOutType Block type when result is BeginDrag.
       *\return Click result.
       */
      PaletteClickResult HandleClick(sf::Vector2f point, BlockType* pOutType);

      /*!
       *\brief Updates hover highlight and active drag ghost position.
       *
       *\param[in] point Mouse position.
       */
      void HandleMouseMove(sf::Vector2f point);

      /*!
       *\brief Clears pressed-row highlight.
       */
      void HandleMouseRelease(void);

      /*!
       *\brief Returns true while a palette block drag is active.
       */
      bool IsBlockDragActive(void) const;

      /*!
       *\brief Returns the block type being dragged, or End if none.
       */
      BlockType GetDragBlockType(void) const;

      /*!
       *\brief Returns the current drag cursor position.
       */
      sf::Vector2f GetDragPoint(void) const;

      /*!
       *\brief Cancels an in-progress block drag.
       */
      void CancelBlockDrag(void);

      /*!
       *\brief Ends a block drag and returns the dragged type.
       *
       *\param[out] pOutType Dragged block type.
       *\return true if a drag was active.
       */
      bool FinishBlockDrag(BlockType* pOutType);

      /*!
       *\brief Scrolls the block list when the wheel is used over this palette.
       *
       *\param[in] delta Wheel delta.
       *\param[in] point Mouse position.
       *\return true if handled.
       */
      bool HandleWheel(float delta, sf::Vector2f point);

      /*!
       *\brief Handles typed characters for the filter field.
       *
       *\param[in] unicode Entered codepoint.
       *\return true if handled.
       */
      bool HandleTextEntered(uint32_t unicode);

      /*!
       *\brief Handles keys for the filter field.
       *
       *\param[in] keyCode Key code.
       *\return true if handled.
       */
      bool HandleKey(sf::Keyboard::Key keyCode);

      /*!
       *\brief Clears filter keyboard focus.
       */
      void BlurFilter(void);

      /*!
       *\brief Returns true when the filter field is focused.
       */
      bool IsFilterFocused(void) const;

      /*!
       *\brief Draws the palette.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

      /*!
       *\brief Draws the in-progress drag ghost above other UI.
       *
       *\param[in,out] pTarget Render target.
       */
      void DrawDragGhost(sf::RenderTarget* pTarget) const;

      /*!
       *\brief Draws the hover help tip above other UI.
       *
       *\param[in,out] pTarget Render target.
       */
      void DrawHoverTip(sf::RenderTarget* pTarget) const;

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
      sf::FloatRect FilterBounds(void) const;
      void ClearHover(void);
      void ClearPressed(void);
      bool FindRowAtPoint(sf::Vector2f point, Row* pOutRow) const;
      bool RowMatchesHighlight(const Row& row,
                               RowKind kind,
                               uint32_t groupIndex,
                               BlockType type) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<bool> _groupExpanded;
      std::vector<Row> _visibleRows;
      uint32_t _scrollRows = 0;
      std::string _filterText;
      bool _filterFocused = false;
      bool _hasHover = false;
      RowKind _hoverKind = RowKind::Block;
      uint32_t _hoverGroupIndex = 0;
      BlockType _hoverType = BlockType::End;
      bool _hasPressed = false;
      RowKind _pressedKind = RowKind::Block;
      uint32_t _pressedGroupIndex = 0;
      BlockType _pressedType = BlockType::End;
      bool _hasSelectedBlock = false;
      BlockType _selectedBlockType = BlockType::End;
      bool _hasSelectedGroup = false;
      uint32_t _selectedGroupIndex = 0;
      sf::Vector2f _hoverPoint {};
      bool _isDraggingBlock = false;
      BlockType _dragBlockType = BlockType::End;
      sf::Vector2f _dragPoint {};
      UiTheme _theme = GetUiTheme(UiThemeId::Dark);
   };
} // namespace Cgen

#endif // PALETTE_H
