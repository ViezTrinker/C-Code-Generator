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
      Help
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
       *\brief Hit-tests a click.
       *
       *\param[in] point Mouse position.
       *\return Action for the clicked button, or None.
       */
      ToolbarAction HitTest(sf::Vector2f point) const;

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
   };
} // namespace Cgen

#endif // TOOLBAR_H
