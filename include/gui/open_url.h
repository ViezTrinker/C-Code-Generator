/*!
 *\file open_url.h
 *\brief Opens http(s) URLs in the system default browser.
 */
#ifndef OPEN_URL_H
#define OPEN_URL_H

#include <string_view>

namespace Cgen
{
   /*!
    *\brief Opens a URL in the system default browser.
    *
    *\param[in] url Absolute http(s) URL.
    *\return true when the OS accepted the open request.
    */
   bool OpenUrlInBrowser(std::string_view url);
} // namespace Cgen

#endif // OPEN_URL_H
