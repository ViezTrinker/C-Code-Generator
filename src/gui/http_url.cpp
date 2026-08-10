/*!
 *\file http_url.cpp
 *\brief http(s) URL extraction from text lines.
 */
#include "gui/http_url.h"

namespace Cgen
{
   namespace
   {
      bool IsUrlTerminator(char character)
      {
         return (character == ' ') || (character == '\t') ||
                (character == '\r') || (character == '\n') ||
                (character == '"') || (character == '\'') ||
                (character == ')') || (character == ']') ||
                (character == '>') || (character == '<');
      }

      bool IsTrailingUrlPunctuation(char character)
      {
         return (character == '.') || (character == ',') ||
                (character == ';') || (character == ':') ||
                (character == '!');
      }
   } // namespace

   bool ExtractHttpUrlFromLine(std::string_view line, std::string* pOutUrl)
   {
      if (pOutUrl == nullptr)
      {
         return false;
      }
      pOutUrl->clear();

      const size_t httpsPos = line.find("https://");
      const size_t httpPos = line.find("http://");
      size_t start = std::string_view::npos;
      if (httpsPos != std::string_view::npos)
      {
         start = httpsPos;
      }
      if ((httpPos != std::string_view::npos) &&
          ((start == std::string_view::npos) || (httpPos < start)))
      {
         start = httpPos;
      }
      if (start == std::string_view::npos)
      {
         return false;
      }

      size_t end = start;
      while ((end < line.size()) && (!IsUrlTerminator(line[end])))
      {
         ++end;
      }
      while ((end > start) && IsTrailingUrlPunctuation(line[end - 1]))
      {
         --end;
      }
      if (end <= start)
      {
         return false;
      }

      *pOutUrl = std::string(line.substr(start, end - start));
      return true;
   }
} // namespace Cgen
