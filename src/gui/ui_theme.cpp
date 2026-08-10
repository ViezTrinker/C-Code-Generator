/*!
 *\file ui_theme.cpp
 *\brief Dark and Light UI color palettes.
 */
#include "gui/ui_theme.h"

namespace Cgen
{
   namespace
   {
      UiTheme MakeDarkTheme(void)
      {
         UiTheme theme;
         theme.id = UiThemeId::Dark;
         theme.windowClear = sf::Color(0, 0, 0);
         theme.panelBackground = sf::Color(18, 18, 18);
         theme.panelOutline = sf::Color(220, 220, 220);
         theme.toolbarBackground = sf::Color(12, 12, 12);

         theme.buttonFill = sf::Color(32, 32, 32);
         theme.buttonHover = sf::Color(55, 55, 55);
         theme.buttonPressed = sf::Color(80, 80, 80);
         theme.buttonActive = sf::Color(70, 70, 70);
         theme.buttonOutline = sf::Color(220, 220, 220);
         theme.buttonActiveOutline = sf::Color(255, 255, 255);

         theme.textPrimary = sf::Color(255, 255, 255);
         theme.textSecondary = sf::Color(230, 230, 230);
         theme.textMuted = sf::Color(170, 170, 170);
         theme.textAccent = sf::Color(240, 240, 240);
         theme.textHelp = sf::Color(190, 190, 190);
         theme.textPreview = sf::Color(210, 210, 255);
         theme.textInput = sf::Color(255, 255, 255);
         theme.textPlaceholder = sf::Color(140, 140, 140);

         theme.inputFill = sf::Color(28, 28, 28);
         theme.inputFillFocused = sf::Color(45, 45, 45);
         theme.inputOutline = sf::Color(220, 220, 220);

         theme.listRow = sf::Color(28, 28, 28);
         theme.listRowHeader = sf::Color(40, 40, 40);
         theme.listRowHover = sf::Color(50, 50, 50);
         theme.listRowHeaderHover = sf::Color(58, 58, 58);
         theme.listRowSelected = sf::Color(70, 70, 70);
         theme.listRowPressed = sf::Color(90, 90, 90);
         theme.listRowOutline = sf::Color(180, 180, 180);
         theme.listRowSelectedOutline = sf::Color(255, 255, 255);

         theme.canvasBackground = sf::Color(0, 0, 0);
         theme.canvasGrid = sf::Color(55, 55, 55);
         theme.nodeFill = sf::Color(28, 28, 28);
         theme.nodeOutline = sf::Color(220, 220, 220);
         theme.nodeSelectedFill = sf::Color(50, 50, 50);
         theme.nodeSelectedOutline = sf::Color(255, 255, 255);
         theme.nodeExpressionFill = sf::Color(24, 36, 28);
         theme.nodeExpressionOutline = sf::Color(160, 200, 170);
         theme.nodeFunctionFill = sf::Color(32, 28, 48);
         theme.nodeFunctionOutline = sf::Color(190, 180, 230);
         theme.functionRegionFill = sf::Color(40, 36, 60, 90);
         theme.functionRegionOutline = sf::Color(180, 170, 220, 160);
         theme.functionHeaderFill = sf::Color(40, 36, 60, 220);
         theme.functionHeaderText = sf::Color(240, 240, 255);
         theme.collapsedHintText = sf::Color(255, 220, 160);

         theme.wireControl = sf::Color(240, 180, 70);
         theme.wireData = sf::Color(80, 180, 255);
         theme.wirePreview = sf::Color(220, 220, 220);
         theme.wireCompatible = sf::Color(90, 220, 130);
         theme.wireIncompatible = sf::Color(230, 90, 90);

         theme.portLabel = sf::Color(210, 210, 210);
         theme.tipBackground = sf::Color(20, 20, 20, 230);
         theme.tipOutline = sf::Color(230, 230, 230);
         theme.tipText = sf::Color(255, 255, 255);
         theme.tipTextOk = sf::Color(180, 255, 200);
         theme.tipTextBad = sf::Color(255, 180, 180);

         theme.marqueeFill = sf::Color(80, 140, 220, 40);
         theme.marqueeOutline = sf::Color(120, 180, 255);

         theme.minimapBackground = sf::Color(12, 12, 12, 220);
         theme.minimapOutline = sf::Color(200, 200, 200);
         theme.minimapNode = sf::Color(200, 200, 200);
         theme.minimapStart = sf::Color(80, 180, 100);
         theme.minimapEnd = sf::Color(200, 90, 90);
         theme.minimapFunction = sf::Color(160, 140, 220);
         theme.minimapViewportFill = sf::Color(90, 160, 255, 45);
         theme.minimapViewportOutline = sf::Color(120, 190, 255);

         theme.popupBackground = sf::Color(24, 24, 24);
         theme.popupOutline = sf::Color(220, 220, 220);
         theme.menuBackground = sf::Color(28, 28, 28);
         theme.menuOutline = sf::Color(220, 220, 220);
         return theme;
      }

      UiTheme MakeLightTheme(void)
      {
         UiTheme theme;
         theme.id = UiThemeId::Light;
         theme.windowClear = sf::Color(255, 255, 255);
         theme.panelBackground = sf::Color(250, 250, 250);
         theme.panelOutline = sf::Color(20, 20, 20);
         theme.toolbarBackground = sf::Color(245, 245, 245);

         theme.buttonFill = sf::Color(255, 255, 255);
         theme.buttonHover = sf::Color(235, 235, 235);
         theme.buttonPressed = sf::Color(210, 210, 210);
         theme.buttonActive = sf::Color(220, 220, 220);
         theme.buttonOutline = sf::Color(20, 20, 20);
         theme.buttonActiveOutline = sf::Color(0, 0, 0);

         theme.textPrimary = sf::Color(0, 0, 0);
         theme.textSecondary = sf::Color(30, 30, 30);
         theme.textMuted = sf::Color(90, 90, 90);
         theme.textAccent = sf::Color(20, 20, 20);
         theme.textHelp = sf::Color(60, 60, 60);
         theme.textPreview = sf::Color(30, 50, 120);
         theme.textInput = sf::Color(0, 0, 0);
         theme.textPlaceholder = sf::Color(120, 120, 120);

         theme.inputFill = sf::Color(255, 255, 255);
         theme.inputFillFocused = sf::Color(245, 245, 245);
         theme.inputOutline = sf::Color(20, 20, 20);

         theme.listRow = sf::Color(255, 255, 255);
         theme.listRowHeader = sf::Color(240, 240, 240);
         theme.listRowHover = sf::Color(230, 230, 230);
         theme.listRowHeaderHover = sf::Color(225, 225, 225);
         theme.listRowSelected = sf::Color(210, 210, 210);
         theme.listRowPressed = sf::Color(190, 190, 190);
         theme.listRowOutline = sf::Color(80, 80, 80);
         theme.listRowSelectedOutline = sf::Color(0, 0, 0);

         theme.canvasBackground = sf::Color(255, 255, 255);
         theme.canvasGrid = sf::Color(200, 200, 200);
         theme.nodeFill = sf::Color(255, 255, 255);
         theme.nodeOutline = sf::Color(0, 0, 0);
         theme.nodeSelectedFill = sf::Color(235, 235, 235);
         theme.nodeSelectedOutline = sf::Color(0, 0, 0);
         theme.nodeExpressionFill = sf::Color(245, 255, 245);
         theme.nodeExpressionOutline = sf::Color(40, 100, 60);
         theme.nodeFunctionFill = sf::Color(245, 245, 255);
         theme.nodeFunctionOutline = sf::Color(70, 50, 140);
         theme.functionRegionFill = sf::Color(230, 230, 245, 90);
         theme.functionRegionOutline = sf::Color(100, 80, 160, 160);
         theme.functionHeaderFill = sf::Color(235, 235, 250, 230);
         theme.functionHeaderText = sf::Color(20, 20, 40);
         theme.collapsedHintText = sf::Color(120, 70, 0);

         theme.wireControl = sf::Color(180, 110, 0);
         theme.wireData = sf::Color(0, 100, 180);
         theme.wirePreview = sf::Color(40, 40, 40);
         theme.wireCompatible = sf::Color(0, 140, 50);
         theme.wireIncompatible = sf::Color(180, 30, 30);

         theme.portLabel = sf::Color(40, 40, 40);
         theme.tipBackground = sf::Color(255, 255, 255, 240);
         theme.tipOutline = sf::Color(0, 0, 0);
         theme.tipText = sf::Color(0, 0, 0);
         theme.tipTextOk = sf::Color(0, 120, 40);
         theme.tipTextBad = sf::Color(160, 0, 0);

         theme.marqueeFill = sf::Color(40, 100, 200, 35);
         theme.marqueeOutline = sf::Color(20, 80, 180);

         theme.minimapBackground = sf::Color(255, 255, 255, 230);
         theme.minimapOutline = sf::Color(0, 0, 0);
         theme.minimapNode = sf::Color(40, 40, 40);
         theme.minimapStart = sf::Color(0, 140, 50);
         theme.minimapEnd = sf::Color(180, 40, 40);
         theme.minimapFunction = sf::Color(90, 60, 160);
         theme.minimapViewportFill = sf::Color(40, 100, 200, 40);
         theme.minimapViewportOutline = sf::Color(20, 80, 180);

         theme.popupBackground = sf::Color(255, 255, 255);
         theme.popupOutline = sf::Color(0, 0, 0);
         theme.menuBackground = sf::Color(255, 255, 255);
         theme.menuOutline = sf::Color(0, 0, 0);
         return theme;
      }

      const UiTheme DarkTheme = MakeDarkTheme();
      const UiTheme LightTheme = MakeLightTheme();
   } // namespace

   const UiTheme& GetUiTheme(UiThemeId themeId)
   {
      if (themeId == UiThemeId::Light)
      {
         return LightTheme;
      }
      return DarkTheme;
   }
} // namespace Cgen
