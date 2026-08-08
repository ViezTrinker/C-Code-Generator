/*!
 *\file log_pane.h
 *\brief Scrollable text log panel for compiler and program output.
 */
#ifndef LOG_PANE_H
#define LOG_PANE_H

#include <string>
#include <string_view>

#include <SFML/Graphics.hpp>

namespace Cgen
{
   /*!
    *\brief Simple multi-line log view.
    */
   class LogPane
   {
   public:
      /*!
       *\brief Constructs a log pane.
       *
       *\param[in] title Pane title.
       *\param[in] font Font used for text.
       */
      LogPane(std::string_view title, const sf::Font& font);

      /*!
       *\brief Sets the screen rectangle for the pane.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Clears log text.
       */
      void Clear(void);

      /*!
       *\brief Appends text to the log.
       *
       *\param[in] text Text to append.
       */
      void Append(std::string_view text);

      /*!
       *\brief Replaces log contents.
       *
       *\param[in] text New text.
       */
      void SetText(std::string_view text);

      /*!
       *\brief Draws the pane.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      std::string _title;
      std::string _text;
      sf::FloatRect _bounds {};
      const sf::Font* _pFont = nullptr;
   };
} // namespace Cgen

#endif // LOG_PANE_H
