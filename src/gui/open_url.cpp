/*!
 *\file open_url.cpp
 *\brief Opens http(s) URLs in the system default browser.
 */
#include "gui/open_url.h"

#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <cstdlib>
#endif

namespace Cgen
{
   bool OpenUrlInBrowser(std::string_view url)
   {
      if (url.empty())
      {
         return false;
      }
      if ((url.find("https://") != 0) && (url.find("http://") != 0))
      {
         return false;
      }

      const std::string urlCopy(url);
#ifdef _WIN32
      const HINSTANCE result =
         ShellExecuteA(nullptr, "open", urlCopy.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      return (reinterpret_cast<INT_PTR>(result) > 32);
#else
      std::string command = "xdg-open \"";
      command.append(urlCopy);
      command.append("\" >/dev/null 2>&1 &");
      return (std::system(command.c_str()) == 0);
#endif
   }
} // namespace Cgen
