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
    *\brief How a property field is edited.
    */
   enum class FieldEditKind: uint8_t
   {
      Text = 0,
      Choice
   };

   /*!
    *\brief Edits string properties of the selected node via keyboard and dropdowns.
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
       * Unchanged selection keeps the active field so typing is not cleared by
       * unrelated mouse-release syncs.
       *
       *\param[in] pDocument Document owning the node.
       *\param[in] selectedNodeId Selected node id, or 0.
       */
      void SetSelection(GraphDocument* pDocument, NodeId selectedNodeId);

      /*!
       *\brief Rebuilds fields from the current document selection.
       *
       * Use after undo/redo when the selected node id is unchanged but property
       * values may have changed.
       */
      void ReloadFromDocument(void);

      /*!
       *\brief Commits the active field and clears keyboard focus.
       */
      void Blur(void);

      /*!
       *\brief Returns true when a property field is accepting keyboard input.
       */
      bool HasKeyboardFocus(void) const;

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
         FieldEditKind editKind = FieldEditKind::Text;
         std::vector<std::string> choices;
         sf::FloatRect bounds {};
      };

      struct ChoiceItem
      {
         std::string label;
         sf::FloatRect bounds {};
      };

      void RebuildFields(void);
      void RebuildHelpLines(std::string_view helpText);
      void RebuildPreviewLines(std::string_view previewText);
      void CommitActiveField(void);
      float FieldsStartY(void) const;
      void CloseChoicePopup(void);
      void OpenChoicePopup(int32_t fieldIndex);
      void RebuildChoicePopupBounds(void);
      bool HandleChoicePopupClick(sf::Vector2f point);
      void FillChoicesForField(Field* pField, BlockType blockType) const;

      const sf::Font* _pFont = nullptr;
      sf::FloatRect _bounds {};
      GraphDocument* _pDocument = nullptr;
      DocumentHistory* _pHistory = nullptr;
      NodeId _selectedNodeId = 0;
      std::vector<Field> _fields;
      std::vector<std::string> _helpLines;
      std::vector<std::string> _previewLines;
      int32_t _activeFieldIndex = -1;
      bool _choicePopupOpen = false;
      int32_t _choicePopupFieldIndex = -1;
      sf::FloatRect _choicePopupBounds {};
      std::vector<ChoiceItem> _choiceItems;
   };
} // namespace Cgen

#endif // PROPERTY_PANEL_H
