/*!
 *\file property_panel.h
 *\brief Right-side property inspector for the selected node.
 */
#ifndef PROPERTY_PANEL_H
#define PROPERTY_PANEL_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

#include "gui/document_history.h"
#include "model/graph_document.h"

namespace Cgen
{
   /*!
    *\brief Edits string properties of the selected node via keyboard.
    */
   class PropertyPanel
   {
   public:
      /*!
       *\brief Constructs the panel.
       *
       *\param[in] font Font for labels.
       */
      explicit PropertyPanel(const sf::Font& font);

      /*!
       *\brief Sets panel bounds.
       *
       *\param[in] bounds Pixel bounds.
       */
      void SetBounds(const sf::FloatRect& bounds);

      /*!
       *\brief Binds undo history used before property commits.
       *
       *\param[in,out] pHistory History pointer, or nullptr.
       */
      void SetHistory(DocumentHistory* pHistory);

      /*!
       *\brief Syncs UI with the selected node.
       *
       *\param[in] pDocument Document owning the node.
       *\param[in] selectedNodeId Selected node id, or 0.
       */
      void SetSelection(GraphDocument* pDocument, NodeId selectedNodeId);

      /*!
       *\brief Handles a mouse click inside the panel.
       *
       *\param[in] point Mouse position.
       *\return true if handled.
       */
      bool HandleClick(sf::Vector2f point);

      /*!
       *\brief Handles text entered events while editing.
       *
       *\param[in] unicode Entered unicode codepoint.
       *\return true if handled.
       */
      bool HandleTextEntered(uint32_t unicode);

      /*!
       *\brief Handles key presses while editing.
       *
       *\param[in] keyCode Key code.
       *\return true if handled.
       */
      bool HandleKey(sf::Keyboard::Key keyCode);

      /*!
       *\brief Draws the panel.
       *
       *\param[in,out] pTarget Render target.
       */
      void Draw(sf::RenderTarget* pTarget) const;

   private:
      struct Field
      {
         std::string key;
         std::string value;
         sf::FloatRect bounds {};
      };

      void RebuildFields(void);
      void RebuildHelpLines(std::string_view helpText);
      void CommitActiveField(void);
      float FieldsStartY(void) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      GraphDocument* _pDocument = nullptr;
      DocumentHistory* _pHistory = nullptr;
      NodeId _selectedNodeId = 0;
      std::vector<Field> _fields;
      std::vector<std::string> _helpLines;
      int32_t _activeFieldIndex = -1;
   };
} // namespace Cgen

#endif // PROPERTY_PANEL_H
