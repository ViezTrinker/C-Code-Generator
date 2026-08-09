/*!
 *\file log_pane.cpp
 *\brief Log pane rendering, stdin line, and auto-scroll.
 */
#include "gui/log_pane.h"

namespace Cgen
{
   namespace
   {
      constexpr unsigned int BodyCharacterSize = 13;
      constexpr float LineHeight = 16.0f;
      constexpr float TitleHeight = 24.0f;
      constexpr float InputHeight = 28.0f;
   } // namespace

   LogPane::LogPane(std::string_view title,
                    const sf::Font& font,
                    LogInputMode inputMode)
      : _title(title)
      , _pFont(&font)
      , _inputMode(inputMode)
   {
   }

   void LogPane::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
      if (_inputMode == LogInputMode::Enabled)
      {
         _inputBounds = sf::FloatRect(
            sf::Vector2f(bounds.position.x + 6.0f,
                         bounds.position.y + bounds.size.y - InputHeight - 6.0f),
            sf::Vector2f(bounds.size.x - 12.0f, InputHeight));
      }
      ClampScroll();
   }

   bool LogPane::Contains(sf::Vector2f point) const
   {
      return _bounds.contains(point);
   }

   sf::FloatRect LogPane::BodyBounds(void) const
   {
      float bottom = _bounds.position.y + _bounds.size.y - 8.0f;
      if (_inputMode == LogInputMode::Enabled)
      {
         bottom = _inputBounds.position.y - 4.0f;
      }
      const float top = _bounds.position.y + TitleHeight;
      float height = bottom - top;
      if (height < LineHeight)
      {
         height = LineHeight;
      }
      return sf::FloatRect(sf::Vector2f(_bounds.position.x + 8.0f, top),
                           sf::Vector2f(_bounds.size.x - 16.0f, height));
   }

   uint32_t LogPane::VisibleLineCapacity(void) const
   {
      const auto height = static_cast<uint32_t>(BodyBounds().size.y / LineHeight);
      if (height < 1)
      {
         return 1;
      }
      return height;
   }

   void LogPane::RebuildLines(void)
   {
      _lines.clear();
      std::string current;
      for (size_t index = 0; index < _text.size(); ++index)
      {
         const char character = _text[index];
         if (character == '\n')
         {
            _lines.push_back(current);
            current.clear();
         }
         else if (character != '\r')
         {
            current.push_back(character);
         }
      }
      if ((!current.empty()) || _lines.empty())
      {
         _lines.push_back(current);
      }
   }

   void LogPane::ClampScroll(void)
   {
      const uint32_t capacity = VisibleLineCapacity();
      const auto lineCount = static_cast<uint32_t>(_lines.size());
      uint32_t maxScroll = 0;
      if (lineCount > capacity)
      {
         maxScroll = lineCount - capacity;
      }
      if (_scrollFromBottom > maxScroll)
      {
         _scrollFromBottom = maxScroll;
      }
      if (_stickToBottom)
      {
         _scrollFromBottom = 0;
      }
   }

   void LogPane::Clear(void)
   {
      _text.clear();
      _lines.clear();
      _lines.push_back(std::string());
      _scrollFromBottom = 0;
      _stickToBottom = true;
   }

   void LogPane::Append(std::string_view text)
   {
      _text.append(text);
      constexpr size_t MaxChars = 100000;
      if (_text.size() > MaxChars)
      {
         _text.erase(0, _text.size() - MaxChars);
      }
      RebuildLines();
      if (_stickToBottom)
      {
         _scrollFromBottom = 0;
      }
      ClampScroll();
   }

   void LogPane::SetText(std::string_view text)
   {
      _text = std::string(text);
      RebuildLines();
      _scrollFromBottom = 0;
      _stickToBottom = true;
      ClampScroll();
   }

   const std::string& LogPane::GetText(void) const
   {
      return _text;
   }

   bool LogPane::HandleClick(sf::Vector2f point)
   {
      if (!_bounds.contains(point))
      {
         return false;
      }
      if (_inputMode == LogInputMode::Enabled)
      {
         _inputFocused = true;
      }
      return true;
   }

   bool LogPane::HandleWheel(float delta, sf::Vector2f point)
   {
      if (!_bounds.contains(point))
      {
         return false;
      }

      const uint32_t capacity = VisibleLineCapacity();
      const auto lineCount = static_cast<uint32_t>(_lines.size());
      uint32_t maxScroll = 0;
      if (lineCount > capacity)
      {
         maxScroll = lineCount - capacity;
      }

      if (delta > 0.0f)
      {
         if (_scrollFromBottom < maxScroll)
         {
            ++_scrollFromBottom;
         }
         _stickToBottom = false;
      }
      else if (delta < 0.0f)
      {
         if (_scrollFromBottom > 0)
         {
            --_scrollFromBottom;
         }
         _stickToBottom = (_scrollFromBottom == 0);
      }
      ClampScroll();
      return true;
   }

   bool LogPane::HandleTextEntered(uint32_t unicode)
   {
      if ((_inputMode != LogInputMode::Enabled) || (!_inputFocused))
      {
         return false;
      }
      if ((unicode == 8) || (unicode == 127))
      {
         if (!_inputLine.empty())
         {
            _inputLine.pop_back();
         }
         return true;
      }
      if ((unicode >= 32) && (unicode < 127))
      {
         _inputLine.push_back(static_cast<char>(unicode));
         return true;
      }
      return false;
   }

   bool LogPane::HandleKey(sf::Keyboard::Key keyCode)
   {
      if ((_inputMode != LogInputMode::Enabled) || (!_inputFocused))
      {
         return false;
      }
      if (keyCode == sf::Keyboard::Key::Enter)
      {
         _pendingInput = _inputLine;
         _inputLine.clear();
         _hasPendingInput = true;
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Escape)
      {
         _inputFocused = false;
         return true;
      }
      if ((keyCode == sf::Keyboard::Key::Backspace) ||
          (keyCode == sf::Keyboard::Key::Delete))
      {
         return true;
      }
      return false;
   }

   void LogPane::FocusInput(void)
   {
      if (_inputMode == LogInputMode::Enabled)
      {
         _inputFocused = true;
      }
   }

   void LogPane::BlurInput(void)
   {
      _inputFocused = false;
   }

   bool LogPane::IsInputFocused(void) const
   {
      return _inputFocused;
   }

   bool LogPane::HasPendingInput(void) const
   {
      return _hasPendingInput;
   }

   std::string LogPane::TakePendingInput(void)
   {
      std::string line = _pendingInput;
      _pendingInput.clear();
      _hasPendingInput = false;
      return line;
   }

   std::string LogPane::BuildVisibleText(void) const
   {
      if (_lines.empty())
      {
         return std::string();
      }

      const uint32_t capacity = VisibleLineCapacity();
      const auto lineCount = static_cast<uint32_t>(_lines.size());
      uint32_t endIndex = lineCount;
      if (_scrollFromBottom < endIndex)
      {
         endIndex = lineCount - _scrollFromBottom;
      }
      uint32_t startIndex = 0;
      if (endIndex > capacity)
      {
         startIndex = endIndex - capacity;
      }

      std::string visible;
      for (uint32_t index = startIndex; index < endIndex; ++index)
      {
         visible.append(_lines[index]);
         visible.push_back('\n');
      }
      return visible;
   }

   void LogPane::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr))
      {
         return;
      }

      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(28, 30, 34));
      background.setOutlineColor(sf::Color(70, 74, 80));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      sf::Text titleText(*_pFont, _title, 14);
      titleText.setFillColor(sf::Color(220, 220, 220));
      titleText.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                         _bounds.position.y + 4.0f));
      pTarget->draw(titleText);

      const sf::FloatRect body = BodyBounds();
      const std::string visible = BuildVisibleText();
      sf::Text bodyText(*_pFont, visible, BodyCharacterSize);
      bodyText.setFillColor(sf::Color(190, 200, 190));
      bodyText.setPosition(sf::Vector2f(body.position.x, body.position.y));
      pTarget->draw(bodyText);

      if (_inputMode == LogInputMode::Enabled)
      {
         sf::RectangleShape inputBox;
         inputBox.setPosition(_inputBounds.position);
         inputBox.setSize(_inputBounds.size);
         if (_inputFocused)
         {
            inputBox.setFillColor(sf::Color(50, 60, 80));
         }
         else
         {
            inputBox.setFillColor(sf::Color(40, 44, 52));
         }
         inputBox.setOutlineColor(sf::Color(100, 120, 150));
         inputBox.setOutlineThickness(1.0f);
         pTarget->draw(inputBox);

         std::string inputDisplay = "> ";
         inputDisplay.append(_inputLine);
         if (_inputFocused)
         {
            inputDisplay.push_back('_');
         }
         sf::Text inputText(*_pFont, inputDisplay, 13);
         inputText.setFillColor(sf::Color(230, 230, 200));
         inputText.setPosition(sf::Vector2f(_inputBounds.position.x + 6.0f,
                                            _inputBounds.position.y + 4.0f));
         pTarget->draw(inputText);
      }
   }
} // namespace Cgen
