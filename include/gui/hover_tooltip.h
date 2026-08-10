/*!
 *\file hover_tooltip.h
 *\brief Shared hover tooltip drawing for palette and toolbar.
 */
#ifndef HOVER_TOOLTIP_H
#define HOVER_TOOLTIP_H

#include <string_view>

#include <SFML/Graphics.hpp>

#include "gui/hover_tooltip_text.h"

namespace Cgen
{
   /*!
    *\brief Draws a wrapped hover tooltip near an anchor point.
    *
    *\param[in,out] pTarget Render target.
    *\param[in] font Font for tooltip text.
    *\param[in] anchor Preferred top-left screen position.
    *\param[in] text Tooltip body.
    *\param[in] maxWidth Maximum text width before wrapping.
    */
   void DrawHoverTooltip(sf::RenderTarget* pTarget,
                         const sf::Font& font,
                         sf::Vector2f anchor,
                         std::string_view text,
                         float maxWidth);
} // namespace Cgen

#endif // HOVER_TOOLTIP_H
