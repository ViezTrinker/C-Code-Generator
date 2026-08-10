/*!
 *\file toolbar.h
 *\brief Top toolbar buttons for file and build actions.
 */
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Graphics.hpp>

#include "gui/toolbar_action.h"
#include "gui/ui_theme.h"

namespace Cgen
{
   /*!
    *\brief Horizontal button toolbar.
    */
   class Toolbar
   {
   public:
      /*!
       *\brief Constructs the toolbar.
       *
       *\param[in] font Font for labels.
       */
      explicit Toolbar(const sf::Font& font);

      /*!
       *\brief Sets toolbar bounds.
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
       *\brief Updates hover highlight from the mouse position.
       *
       *\param[in] point Mouse position.
       */
      void HandleMouseMove(sf::Vector2f point);

      /*!
       *\brief Clears pressed-button highlight.
       */
      void HandleMouseRelease(void);

      /*!
       *\brief Hit-tests a click and marks the button as pressed/active.
       *
       *\param[in] point Mouse position.
       *\return Action for the clicked button, or None.
       */
      ToolbarAction HitTest(sf::Vector2f point);

      /*!
       *\brief Draws the toolbar.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

      /*!
       *\brief Draws the hover tooltip above other UI.
       *
       *\param[in,out] pTarget Render target.
       */
      void DrawHoverTip(sf::RenderTarget* pTarget) const;

   private:
      struct Button
      {
         ToolbarAction action = ToolbarAction::None;
         std::string label;
         std::string tooltip;
         sf::FloatRect bounds {};
      };

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<Button> _buttons;
      ToolbarAction _hoveredAction = ToolbarAction::None;
      ToolbarAction _pressedAction = ToolbarAction::None;
      ToolbarAction _activeAction = ToolbarAction::None;
      sf::Vector2f _hoverPoint {};
      UiTheme _theme = GetUiTheme(UiThemeId::Dark);
   };
} // namespace Cgen

#endif // TOOLBAR_H
