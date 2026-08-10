/*!
 *\file app_version.h
 *\brief Application version and author metadata for About UI.
 */
#ifndef APP_VERSION_H
#define APP_VERSION_H

#include <string_view>

namespace Cgen
{
   inline constexpr std::string_view AppProductName =
      "Graphical C Code Generator";
   inline constexpr std::string_view AppVersionLabel = "1.5.0.R";
   inline constexpr std::string_view AppReleaseDate = "10.08.2026";
   inline constexpr std::string_view AppAuthorName = "ViezTrinker";
   inline constexpr std::string_view AppAuthorProfileUrl =
      "https://github.com/ViezTrinker";
   inline constexpr std::string_view AppRepositoryUrl =
      "https://github.com/ViezTrinker/Graphical-C-Code-Generator";
} // namespace Cgen

#endif // APP_VERSION_H
