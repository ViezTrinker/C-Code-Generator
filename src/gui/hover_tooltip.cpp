/*!
 *\file hover_tooltip.cpp
 *\brief Shared hover tooltip drawing for palette and toolbar.
 */
#include "gui/hover_tooltip.h"

#include <string>
#include <vector>

namespace Cgen
{
   namespace
   {
      constexpr unsigned int TooltipCharacterSize = 12;
      constexpr float TooltipLineHeight = 16.0f;
      constexpr float TooltipPadX = 8.0f;
      constexpr float TooltipPadY = 6.0f;
      constexpr float TooltipGap = 10.0f;
   } // namespace

   void DrawHoverTooltip(sf::RenderTarget* pTarget,
                         const sf::Font& font,
                         sf::Vector2f anchor,
                         std::string_view text,
                         float maxWidth)
   {
      if ((pTarget == nullptr) || text.empty())
      {
         return;
      }

      std::vector<std::string> lines;
      WrapHoverTooltipLines(text, maxWidth, &lines);
      if (lines.empty())
      {
         return;
      }

      float contentWidth = 0.0f;
      for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
      {
         sf::Text measure(font, lines[lineIndex], TooltipCharacterSize);
         const sf::FloatRect bounds = measure.getLocalBounds();
         if (bounds.size.x > contentWidth)
         {
            contentWidth = bounds.size.x;
         }
      }

      const float boxWidth = contentWidth + (TooltipPadX * 2.0f);
      const float boxHeight =
         (static_cast<float>(lines.size()) * TooltipLineHeight) +
         (TooltipPadY * 2.0f);

      sf::Vector2f position(anchor.x + TooltipGap, anchor.y + TooltipGap);
      const sf::Vector2u targetSize = pTarget->getSize();
      if (targetSize.x > 0)
      {
         const float maxX = static_cast<float>(targetSize.x) - boxWidth - 4.0f;
         if (position.x > maxX)
         {
            position.x = maxX;
         }
         if (position.x < 4.0f)
         {
            position.x = 4.0f;
         }
      }
      if (targetSize.y > 0)
      {
         const float maxY = static_cast<float>(targetSize.y) - boxHeight - 4.0f;
         if (position.y > maxY)
         {
            position.y = maxY;
         }
         if (position.y < 4.0f)
         {
            position.y = 4.0f;
         }
      }

      sf::RectangleShape background;
      background.setPosition(position);
      background.setSize(sf::Vector2f(boxWidth, boxHeight));
      background.setFillColor(sf::Color(20, 20, 28, 230));
      background.setOutlineColor(sf::Color(160, 175, 200));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      float textY = position.y + TooltipPadY;
      for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
      {
         sf::Text line(font, lines[lineIndex], TooltipCharacterSize);
         line.setFillColor(sf::Color(230, 235, 245));
         line.setPosition(sf::Vector2f(position.x + TooltipPadX, textY));
         pTarget->draw(line);
         textY += TooltipLineHeight;
      }
   }
} // namespace Cgen
