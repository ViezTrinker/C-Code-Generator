/*!
 *\file http_url.h
 *\brief Helpers to extract http(s) URLs from UI text.
 */
#ifndef HTTP_URL_H
#define HTTP_URL_H

#include <string>
#include <string_view>

namespace Cgen
{
   /*!
    *\brief Finds the first http:// or https:// URL in a line of text.
    *
    *\param[in] line Text that may contain a URL.
    *\param[out] pOutUrl Receives the extracted URL when found.
    *\return true when a URL was written to pOutUrl.
    */
   bool ExtractHttpUrlFromLine(std::string_view line, std::string* pOutUrl);
} // namespace Cgen

#endif // HTTP_URL_H
