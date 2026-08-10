/*!
 *\file ui_theme_id.cpp
 *\brief UI theme identifier helpers.
 */
#include "gui/ui_theme_id.h"

namespace Cgen
{
   UiThemeId CycleUiThemeId(UiThemeId themeId)
   {
      if (themeId == UiThemeId::Dark)
      {
         return UiThemeId::Light;
      }
      return UiThemeId::Dark;
   }

   std::string_view UiThemeIdToString(UiThemeId themeId)
   {
      if (themeId == UiThemeId::Light)
      {
         return "light";
      }
      return "dark";
   }

   bool UiThemeIdFromString(std::string_view text, UiThemeId* pOutThemeId)
   {
      if (pOutThemeId == nullptr)
      {
         return false;
      }
      if (text == "light")
      {
         *pOutThemeId = UiThemeId::Light;
         return true;
      }
      if (text == "dark")
      {
         *pOutThemeId = UiThemeId::Dark;
         return true;
      }
      return false;
   }
} // namespace Cgen
