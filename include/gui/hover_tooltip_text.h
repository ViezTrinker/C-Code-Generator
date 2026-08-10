/*!
 *\file hover_tooltip_text.h
 *\brief Tooltip text helpers for palette and toolbar hover tips.
 */
#ifndef HOVER_TOOLTIP_TEXT_H
#define HOVER_TOOLTIP_TEXT_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "gui/toolbar_action.h"
#include "model/block_type.h"

namespace Cgen
{
   /*!
    *\brief Whether a palette row is a group header or a block entry.
    */
   enum class PaletteRowTipKind: uint8_t
   {
      GroupHeader = 0,
      Block = 1
   };

   /*!
    *\brief Wraps tooltip text into lines for a target pixel width.
    *
    *\param[in] text Source text.
    *\param[in] maxWidth Approximate max line width in pixels.
    *\param[out] pOutLines Receives wrapped lines (cleared first).
    */
   void WrapHoverTooltipLines(std::string_view text,
                              float maxWidth,
                              std::vector<std::string>* pOutLines);

   /*!
    *\brief Returns hover tip text for a palette row.
    *
    * Block rows use the same help string as the Properties panel.
    *
    *\param[in] tipKind Group header or block.
    *\param[in] blockType Block type when tipKind is Block.
    */
   std::string_view PaletteRowHoverTipText(PaletteRowTipKind tipKind,
                                           BlockType blockType);

   /*!
    *\brief Returns hover tip text for a toolbar action.
    *
    *\param[in] action Toolbar action, or None for empty.
    */
   std::string_view ToolbarActionTooltipText(ToolbarAction action);
} // namespace Cgen

#endif // HOVER_TOOLTIP_TEXT_H
