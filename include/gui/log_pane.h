/*!
 *\file log_pane.h
 *\brief Scrollable text log panel for compiler and program output.
 */
#ifndef LOG_PANE_H
#define LOG_PANE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace Cgen
{
   /*!
    *\brief Whether the pane shows a stdin input line.
    */
   enum class LogInputMode: bool
   {
      Disabled = false,
      Enabled = true
   };

   /*!
    *\brief Simple multi-line log view with optional stdin line and scrolling.
    */
   class LogPane
   {
   public:
      /*!
       *\brief Constructs a log pane.
       *
       *\param[in] title Pane title.
       *\param[in] font Font used for text.
       *\param[in] inputMode Whether to show an input line.
       */
      LogPane(std::string_view title,
              const sf::Font& font,
              LogInputMode inputMode);

      /*!
       *\brief Sets the screen rectangle for the pane.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Returns true if the point lies inside the pane bounds.
       *
       *\param[in] point Mouse position.
       */
      bool Contains(sf::Vector2f point) const;

      /*!
       *\brief Clears log text and resets scroll to the bottom.
       */
      void Clear(void);

      /*!
       *\brief Appends text to the log (keeps auto-scroll if already at bottom).
       *
       *\param[in] text Text to append.
       */
      void Append(std::string_view text);

      /*!
       *\brief Replaces log contents and scrolls to the bottom.
       *
       *\param[in] text New text.
       */
      void SetText(std::string_view text);

      /*!
       *\brief Returns the current log body text.
       */
      const std::string& GetText(void) const;

      /*!
       *\brief Focuses the input line if enabled.
       *
       *\param[in] point Click position.
       *\return true if the click was handled.
       */
      bool HandleClick(sf::Vector2f point);

      /*!
       *\brief Scrolls the log when the wheel is used over this pane.
       *
       *\param[in] delta Wheel delta.
       *\param[in] point Mouse position.
       *\return true if handled.
       */
      bool HandleWheel(float delta, sf::Vector2f point);

      /*!
       *\brief Handles typed characters for the input line.
       *
       *\param[in] unicode Entered codepoint.
       *\return true if handled.
       */
      bool HandleTextEntered(uint32_t unicode);

      /*!
       *\brief Handles keys for the input line.
       *
       *\param[in] keyCode Key code.
       *\return true if handled.
       */
      bool HandleKey(sf::Keyboard::Key keyCode);

      /*!
       *\brief Returns true when Enter submitted a stdin line.
       */
      bool HasPendingInput(void) const;

      /*!
       *\brief Takes the submitted stdin line (without trailing newline).
       */
      std::string TakePendingInput(void);

      /*!
       *\brief Draws the pane.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      void RebuildLines(void);
      void ClampScroll(void);
      uint32_t VisibleLineCapacity(void) const;
      sf::FloatRect BodyBounds(void) const;
      std::string BuildVisibleText(void) const;

      std::string _title;
      std::string _text;
      std::vector<std::string> _lines;
      std::string _inputLine;
      std::string _pendingInput;
      bool _hasPendingInput = false;
      sf::FloatRect _bounds {};
      sf::FloatRect _inputBounds {};
      const sf::Font* _pFont = nullptr;
      LogInputMode _inputMode = LogInputMode::Disabled;
      bool _inputFocused = false;
      /*!
       *\brief Lines hidden below the viewport (0 = pinned to latest output).
       */
      uint32_t _scrollFromBottom = 0;
      bool _stickToBottom = true;
   };
} // namespace Cgen

#endif // LOG_PANE_H
