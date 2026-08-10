/*!
 *\file ui_theme_id.h
 *\brief UI theme identifiers (no SFML dependency).
 */
#ifndef UI_THEME_ID_H
#define UI_THEME_ID_H

#include <cstdint>
#include <string_view>

namespace Cgen
{
   /*!
    *\brief Available application color themes.
    */
   enum class UiThemeId: uint8_t
   {
      Dark = 0,
      Light = 1
   };

   /*!
    *\brief Cycles Dark → Light → Dark.
    *
    *\param[in] themeId Current theme.
    */
   UiThemeId CycleUiThemeId(UiThemeId themeId);

   /*!
    *\brief Stable string id for persistence ("dark" / "light").
    *
    *\param[in] themeId Theme id.
    */
   std::string_view UiThemeIdToString(UiThemeId themeId);

   /*!
    *\brief Parses a theme id string.
    *
    *\param[in] text Theme name.
    *\param[out] pOutThemeId Receives parsed id.
    *\return true on success.
    */
   bool UiThemeIdFromString(std::string_view text, UiThemeId* pOutThemeId);
} // namespace Cgen

#endif // UI_THEME_ID_H
