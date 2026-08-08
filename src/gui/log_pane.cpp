/*!
 *\file log_pane.cpp
 *\brief Log pane rendering.
 */
#include "gui/log_pane.h"

namespace Cgen
{
   LogPane::LogPane(std::string_view title, const sf::Font& font)
      : _title(title)
      , _pFont(&font)
   {
   }

   void LogPane::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
   }

   void LogPane::Clear(void)
   {
      _text.clear();
   }

   void LogPane::Append(std::string_view text)
   {
      _text.append(text);
   }

   void LogPane::SetText(std::string_view text)
   {
      _text = std::string(text);
   }

   void LogPane::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }

      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(28, 30, 34));
      background.setOutlineColor(sf::Color(70, 74, 80));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      sf::Text titleText(*_pFont, _title, 14);
      titleText.setFillColor(sf::Color(220, 220, 220));
      titleText.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                         _bounds.position.y + 4.0f));
      pTarget->draw(titleText);

      sf::Text bodyText(*_pFont, _text, 13);
      bodyText.setFillColor(sf::Color(190, 200, 190));
      bodyText.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                        _bounds.position.y + 24.0f));
      pTarget->draw(bodyText);
   }
} // namespace Cgen
