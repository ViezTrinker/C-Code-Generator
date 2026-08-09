/*!
 *\file toolbar.h
 *\brief Top toolbar buttons for file and build actions.
 */
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Graphics.hpp>

namespace Cgen
{
   /*!
    *\brief Toolbar action identifiers.
    */
   enum class ToolbarAction: uint8_t
   {
      None = 0,
      NewDocument,
      Open,
      Save,
      Generate,
      Build,
      Run,
      Stop,
      Tidy,
      Help,
      FitAll,
      FitSelection,
      Snap,
      AlignLeft,
      AlignTop
   };

   /*!
    *\brief Horizontal button toolbar.
    */
   class Toolbar
   {
   public:
      /*!
       *\brief Constructs the toolbar.
       *
       *\param[in] font Font for labels.
       */
      explicit Toolbar(const sf::Font& font);

      /*!
       *\brief Sets toolbar bounds.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Updates hover highlight from the mouse position.
       *
       *\param[in] point Mouse position.
       */
      void HandleMouseMove(sf::Vector2f point);

      /*!
       *\brief Clears pressed-button highlight.
       */
      void HandleMouseRelease(void);

      /*!
       *\brief Hit-tests a click and marks the button as pressed/active.
       *
       *\param[in] point Mouse position.
       *\return Action for the clicked button, or None.
       */
      ToolbarAction HitTest(sf::Vector2f point);

      /*!
       *\brief Draws the toolbar.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      struct Button
      {
         ToolbarAction action = ToolbarAction::None;
         std::string label;
         sf::FloatRect bounds {};
      };

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      std::vector<Button> _buttons;
      ToolbarAction _hoveredAction = ToolbarAction::None;
      ToolbarAction _pressedAction = ToolbarAction::None;
      ToolbarAction _activeAction = ToolbarAction::None;
   };
} // namespace Cgen

#endif // TOOLBAR_H
