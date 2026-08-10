/*!
 *\file ui_theme.h
 *\brief Concrete SFML color palettes for Dark and Light UI themes.
 */
#ifndef UI_THEME_H
#define UI_THEME_H

#include <SFML/Graphics/Color.hpp>

#include "gui/ui_theme_id.h"

namespace Cgen
{
   /*!
    *\brief Color tokens shared by GUI panels.
    */
   struct UiTheme
   {
      UiThemeId id = UiThemeId::Dark;

      sf::Color windowClear {};

      sf::Color panelBackground {};
      sf::Color panelOutline {};
      sf::Color toolbarBackground {};

      sf::Color buttonFill {};
      sf::Color buttonHover {};
      sf::Color buttonPressed {};
      sf::Color buttonActive {};
      sf::Color buttonOutline {};
      sf::Color buttonActiveOutline {};

      sf::Color textPrimary {};
      sf::Color textSecondary {};
      sf::Color textMuted {};
      sf::Color textAccent {};
      sf::Color textHelp {};
      sf::Color textPreview {};
      sf::Color textInput {};
      sf::Color textPlaceholder {};

      sf::Color inputFill {};
      sf::Color inputFillFocused {};
      sf::Color inputOutline {};

      sf::Color listRow {};
      sf::Color listRowHeader {};
      sf::Color listRowHover {};
      sf::Color listRowHeaderHover {};
      sf::Color listRowSelected {};
      sf::Color listRowPressed {};
      sf::Color listRowOutline {};
      sf::Color listRowSelectedOutline {};

      sf::Color canvasBackground {};
      sf::Color canvasGrid {};
      sf::Color nodeFill {};
      sf::Color nodeOutline {};
      sf::Color nodeSelectedFill {};
      sf::Color nodeSelectedOutline {};
      sf::Color nodeExpressionFill {};
      sf::Color nodeExpressionOutline {};
      sf::Color nodeFunctionFill {};
      sf::Color nodeFunctionOutline {};
      sf::Color functionRegionFill {};
      sf::Color functionRegionOutline {};
      sf::Color functionHeaderFill {};
      sf::Color functionHeaderText {};
      sf::Color collapsedHintText {};

      sf::Color wireControl {};
      sf::Color wireData {};
      sf::Color wirePreview {};
      sf::Color wireCompatible {};
      sf::Color wireIncompatible {};

      sf::Color portLabel {};
      sf::Color tipBackground {};
      sf::Color tipOutline {};
      sf::Color tipText {};
      sf::Color tipTextOk {};
      sf::Color tipTextBad {};

      sf::Color marqueeFill {};
      sf::Color marqueeOutline {};

      sf::Color minimapBackground {};
      sf::Color minimapOutline {};
      sf::Color minimapNode {};
      sf::Color minimapStart {};
      sf::Color minimapEnd {};
      sf::Color minimapFunction {};
      sf::Color minimapViewportFill {};
      sf::Color minimapViewportOutline {};

      sf::Color popupBackground {};
      sf::Color popupOutline {};
      sf::Color menuBackground {};
      sf::Color menuOutline {};
   };

   /*!
    *\brief Returns the palette for a theme id.
    *
    *\param[in] themeId Theme id.
    */
   const UiTheme& GetUiTheme(UiThemeId themeId);
} // namespace Cgen

#endif // UI_THEME_H
