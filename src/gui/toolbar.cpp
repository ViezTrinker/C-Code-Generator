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
      constexpr float TooltipMaxWidth = 280.0f;
      constexpr unsigned int LabelCharacterSize = 14;
      constexpr float ButtonHeight = 28.0f;
      constexpr float ButtonGap = 4.0f;
      constexpr float ButtonPadX = 20.0f;
      constexpr float ButtonMinWidth = 36.0f;
      constexpr float SidePad = 8.0f;

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

      bool IsRightPinnedAction(ToolbarAction action)
      {
         return (action == ToolbarAction::Theme) ||
                (action == ToolbarAction::About) ||
                (action == ToolbarAction::Help);
      }

      std::string ThemeButtonLabel(UiThemeId themeId)
      {
         if (themeId == UiThemeId::Light)
         {
            return "Light";
         }
         return "Dark";
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
         {ToolbarAction::OrthogonalWires, "Ortho",
          std::string(ToolbarActionTooltipText(ToolbarAction::OrthogonalWires)),
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
         {ToolbarAction::Theme, ThemeButtonLabel(UiThemeId::Dark),
          std::string(ToolbarActionTooltipText(ToolbarAction::Theme)),
          {}});
      _buttons.push_back(
         {ToolbarAction::About, "About",
          std::string(ToolbarActionTooltipText(ToolbarAction::About)),
          {}});
      _buttons.push_back(
         {ToolbarAction::Help, "?",
          std::string(ToolbarActionTooltipText(ToolbarAction::Help)),
          {}});
   }

   void Toolbar::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      const float cursorY = bounds.position.y + 6.0f;

      float rightPinnedWidth = 0.0f;
      uint32_t rightPinnedCount = 0;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (!IsRightPinnedAction(_buttons[index].action))
         {
            continue;
         }
         float width = ButtonMinWidth;
         if (_pFont != nullptr)
         {
            width = MeasureButtonWidth(*_pFont, _buttons[index].label);
         }
         rightPinnedWidth += width;
         ++rightPinnedCount;
      }
      if (rightPinnedCount > 1)
      {
         rightPinnedWidth +=
            ButtonGap * static_cast<float>(rightPinnedCount - 1);
      }

      float rightCursorX =
         bounds.position.x + bounds.size.x - SidePad - rightPinnedWidth;
      float leftCursorX = bounds.position.x + SidePad;

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         float width = ButtonMinWidth;
         if (_pFont != nullptr)
         {
            width = MeasureButtonWidth(*_pFont, _buttons[index].label);
         }

         if (IsRightPinnedAction(_buttons[index].action))
         {
            _buttons[index].bounds =
               sf::FloatRect(sf::Vector2f(rightCursorX, cursorY),
                             sf::Vector2f(width, ButtonHeight));
            rightCursorX += width + ButtonGap;
            continue;
         }

         _buttons[index].bounds =
            sf::FloatRect(sf::Vector2f(leftCursorX, cursorY),
                          sf::Vector2f(width, ButtonHeight));
         leftCursorX += width + ButtonGap;
      }
   }

   void Toolbar::SetTheme(const UiTheme& theme)
   {
      _theme = theme;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (_buttons[index].action != ToolbarAction::Theme)
         {
            continue;
         }
         _buttons[index].label = ThemeButtonLabel(theme.id);
         _buttons[index].tooltip =
            std::string(ToolbarActionTooltipText(ToolbarAction::Theme));
         break;
      }
   }

   void Toolbar::HandleMouseMove(sf::Vector2f point)
   {
      _hoverPoint = point;
      _hoveredAction = ToolbarAction::None;
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (!IsRightPinnedAction(_buttons[index].action))
         {
            continue;
         }
         if (!_buttons[index].bounds.contains(point))
         {
            continue;
         }
         _hoveredAction = _buttons[index].action;
         return;
      }
      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (IsRightPinnedAction(_buttons[index].action))
         {
            continue;
         }
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
      if (!_bounds.contains(point))
      {
         return ToolbarAction::None;
      }

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (!IsRightPinnedAction(_buttons[index].action))
         {
            continue;
         }
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

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         if (IsRightPinnedAction(_buttons[index].action))
         {
            continue;
         }
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
      background.setFillColor(_theme.toolbarBackground);
      pTarget->draw(background);

      for (size_t index = 0; index < _buttons.size(); ++index)
      {
         const Button& button = _buttons[index];
         sf::RectangleShape shape;
         shape.setPosition(button.bounds.position);
         shape.setSize(button.bounds.size);

         sf::Color fillColor = _theme.buttonFill;
         sf::Color outlineColor = _theme.buttonOutline;
         if (button.action == _pressedAction)
         {
            fillColor = _theme.buttonPressed;
            outlineColor = _theme.buttonActiveOutline;
         }
         else if (button.action == _activeAction)
         {
            fillColor = _theme.buttonActive;
            outlineColor = _theme.buttonActiveOutline;
         }
         else if (button.action == _hoveredAction)
         {
            fillColor = _theme.buttonHover;
         }

         shape.setFillColor(fillColor);
         shape.setOutlineColor(outlineColor);
         shape.setOutlineThickness(1.0f);
         pTarget->draw(shape);

         sf::Text label(*_pFont, button.label, LabelCharacterSize);
         label.setFillColor(_theme.textPrimary);
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
                          TooltipMaxWidth,
                          _theme);
         return;
      }
   }
} // namespace Cgen
