/*!
 *\file toolbar.cpp
 *\brief Toolbar implementation.
 */
#include "gui/toolbar.h"

namespace Cgen
{
   Toolbar::Toolbar(const sf::Font& font)
      : _pFont(&font)
   {
      _buttons.push_back({ToolbarAction::NewDocument, "New", {}});
      _buttons.push_back({ToolbarAction::Open, "Open", {}});
      _buttons.push_back({ToolbarAction::Save, "Save", {}});
      _buttons.push_back({ToolbarAction::Generate, "Generate C", {}});
      _buttons.push_back({ToolbarAction::Build, "Build", {}});
      _buttons.push_back({ToolbarAction::Run, "Run", {}});
      _buttons.push_back({ToolbarAction::Stop, "Stop", {}});
      _buttons.push_back({ToolbarAction::Tidy, "Tidy", {}});
      _buttons.push_back({ToolbarAction::FitAll, "Fit", {}});
      _buttons.push_back({ToolbarAction::FitSelection, "Fit Sel", {}});
      _buttons.push_back({ToolbarAction::Help, "?", {}});
   }

   void Toolbar::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      constexpr float ButtonWidth = 72.0f;
      constexpr float HelpButtonWidth = 36.0f;
      constexpr float ButtonHeight = 28.0f;
      constexpr float Gap = 4.0f;
      float cursorX = bounds.position.x + 8.0f;
      const float cursorY = bounds.position.y + 6.0f;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         const float width =
            (_buttons[index].action == ToolbarAction::Help) ? HelpButtonWidth
                                                            : ButtonWidth;
         _buttons[index].bounds = sf::FloatRect(sf::Vector2f(cursorX, cursorY),
                                                sf::Vector2f(width, ButtonHeight));
         cursorX += width + Gap;
      }
   }

   ToolbarAction Toolbar::HitTest(sf::Vector2f point) const
   {
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (_buttons[index].bounds.contains(point))
         {
            return _buttons[index].action;
         }
      }
      return ToolbarAction::None;
   }

   void Toolbar::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }
      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(40, 44, 52));
      pTarget->draw(background);

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         const Button& button = _buttons[index];
         sf::RectangleShape shape;
         shape.setPosition(button.bounds.position);
         shape.setSize(button.bounds.size);
         shape.setFillColor(sf::Color(60, 66, 78));
         shape.setOutlineColor(sf::Color(100, 110, 130));
         shape.setOutlineThickness(1.0f);
         pTarget->draw(shape);

         sf::Text label(*_pFont, button.label, 14);
         label.setFillColor(sf::Color::White);
         label.setPosition(sf::Vector2f(button.bounds.position.x + 10.0f,
                                        button.bounds.position.y + 4.0f));
         pTarget->draw(label);
      }
   }
} // namespace Cgen
