/*!
 *\file toolbar.cpp
 *\brief Toolbar implementation.
 */
#include "gui/toolbar.h"

#include "gui/hover_tooltip.h"
#include "gui/hover_tooltip_text.h"

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
      constexpr float TooltipMaxWidth = 280.0f;
      constexpr unsigned int LabelCharacterSize = 14;
      constexpr float ButtonHeight = 28.0f;
      constexpr float ButtonGap = 4.0f;
      constexpr float ButtonPadX = 20.0f;
      constexpr float ButtonMinWidth = 36.0f;

      float MeasureButtonWidth(const sf::Font& font, std::string_view label)
      {
         sf::Text measure(font, std::string(label), LabelCharacterSize);
         const float textWidth = measure.getLocalBounds().size.x;
         const float width = textWidth + ButtonPadX;
         if (width < ButtonMinWidth)
         {
            return ButtonMinWidth;
         }
         return width;
      }
   } // namespace

   Toolbar::Toolbar(const sf::Font& font)
      : _pFont(&font)
   {
      _buttons.push_back(
         {ToolbarAction::NewDocument, "New",
          std::string(ToolbarActionTooltipText(ToolbarAction::NewDocument)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Open, "Open",
          std::string(ToolbarActionTooltipText(ToolbarAction::Open)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Save, "Save",
          std::string(ToolbarActionTooltipText(ToolbarAction::Save)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Generate, "Generate C",
          std::string(ToolbarActionTooltipText(ToolbarAction::Generate)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Build, "Build",
          std::string(ToolbarActionTooltipText(ToolbarAction::Build)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Run, "Run",
          std::string(ToolbarActionTooltipText(ToolbarAction::Run)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Stop, "Stop",
          std::string(ToolbarActionTooltipText(ToolbarAction::Stop)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Tidy, "Tidy",
          std::string(ToolbarActionTooltipText(ToolbarAction::Tidy)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Snap, "Snap",
          std::string(ToolbarActionTooltipText(ToolbarAction::Snap)),
          {}});
      _buttons.push_back(
         {ToolbarAction::AlignLeft, "AlignL",
          std::string(ToolbarActionTooltipText(ToolbarAction::AlignLeft)),
          {}});
      _buttons.push_back(
         {ToolbarAction::AlignTop, "AlignT",
          std::string(ToolbarActionTooltipText(ToolbarAction::AlignTop)),
          {}});
      _buttons.push_back(
         {ToolbarAction::FitAll, "Fit",
          std::string(ToolbarActionTooltipText(ToolbarAction::FitAll)),
          {}});
      _buttons.push_back(
         {ToolbarAction::FitSelection, "Fit Sel",
          std::string(ToolbarActionTooltipText(ToolbarAction::FitSelection)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Help, "?",
          std::string(ToolbarActionTooltipText(ToolbarAction::Help)),
          {}});
   }

   void Toolbar::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      float cursorX = bounds.position.x + 8.0f;
      const float cursorY = bounds.position.y + 6.0f;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         float width = ButtonMinWidth;
         if (_pFont != nullptr)
         {
            width = MeasureButtonWidth(*_pFont, _buttons[index].label);
         }
         _buttons[index].bounds = sf::FloatRect(sf::Vector2f(cursorX, cursorY),
                                                sf::Vector2f(width, ButtonHeight));
         cursorX += width + ButtonGap;
      }
   }

   void Toolbar::HandleMouseMove(sf::Vector2f point)
   {
      _hoverPoint = point;
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
         _hoverPoint = point;
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

         sf::Text label(*_pFont, button.label, LabelCharacterSize);
         label.setFillColor(sf::Color::White);
         const sf::FloatRect textBounds = label.getLocalBounds();
         const float textX =
            button.bounds.position.x +
            ((button.bounds.size.x - textBounds.size.x) * 0.5f) -
            textBounds.position.x;
         const float textY =
            button.bounds.position.y +
            ((button.bounds.size.y - textBounds.size.y) * 0.5f) -
            textBounds.position.y;
         label.setPosition(sf::Vector2f(textX, textY));
         pTarget->draw(label);
      }
   }

   void Toolbar::DrawHoverTip(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr) ||
          (_hoveredAction == ToolbarAction::None))
      {
         return;
      }
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (_buttons[index].action != _hoveredAction)
         {
            continue;
         }
         DrawHoverTooltip(pTarget,
                          *_pFont,
                          _hoverPoint,
                          _buttons[index].tooltip,
                          TooltipMaxWidth);
         return;
      }
   }
} // namespace Cgen
