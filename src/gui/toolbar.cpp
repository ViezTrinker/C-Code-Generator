/*!
 *\file toolbar.cpp
 *\brief Toolbar implementation.
 */
#include "gui/toolbar.h"

namespace Cgen
{
   namespace
   {
      const sf::Color BackgroundColor(40, 44, 52);
      const sf::Color ButtonNormalFill(60, 66, 78);
      const sf::Color ButtonHoverFill(78, 90, 112);
      const sf::Color ButtonPressedFill(70, 110, 170);
      const sf::Color ButtonActiveFill(55, 95, 145);
      const sf::Color ButtonOutline(100, 110, 130);
      const sf::Color ButtonActiveOutline(150, 190, 240);
   } // namespace

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
      _buttons.push_back({ToolbarAction::Snap, "Snap", {}});
      _buttons.push_back({ToolbarAction::AlignLeft, "AlignL", {}});
      _buttons.push_back({ToolbarAction::AlignTop, "AlignT", {}});
      _buttons.push_back({ToolbarAction::FitAll, "Fit", {}});
      _buttons.push_back({ToolbarAction::FitSelection, "Fit Sel", {}});
      _buttons.push_back({ToolbarAction::Help, "?", {}});
   }

   void Toolbar::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      constexpr float ButtonWidth = 64.0f;
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

   void Toolbar::HandleMouseMove(sf::Vector2f point)
   {
      _hoveredAction = ToolbarAction::None;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (!_buttons[index].bounds.contains(point))
         {
            continue;
         }
         _hoveredAction = _buttons[index].action;
         return;
      }
   }

   void Toolbar::HandleMouseRelease(void)
   {
      _pressedAction = ToolbarAction::None;
   }

   ToolbarAction Toolbar::HitTest(sf::Vector2f point)
   {
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (!_buttons[index].bounds.contains(point))
         {
            continue;
         }
         _pressedAction = _buttons[index].action;
         _activeAction = _buttons[index].action;
         _hoveredAction = _buttons[index].action;
         return _buttons[index].action;
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
      background.setFillColor(BackgroundColor);
      pTarget->draw(background);

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         const Button& button = _buttons[index];
         sf::RectangleShape shape;
         shape.setPosition(button.bounds.position);
         shape.setSize(button.bounds.size);

         sf::Color fillColor = ButtonNormalFill;
         sf::Color outlineColor = ButtonOutline;
         if (button.action == _pressedAction)
         {
            fillColor = ButtonPressedFill;
            outlineColor = ButtonActiveOutline;
         }
         else if (button.action == _activeAction)
         {
            fillColor = ButtonActiveFill;
            outlineColor = ButtonActiveOutline;
         }
         else if (button.action == _hoveredAction)
         {
            fillColor = ButtonHoverFill;
         }

         shape.setFillColor(fillColor);
         shape.setOutlineColor(outlineColor);
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
