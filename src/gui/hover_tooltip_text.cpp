/*!
 *\file hover_tooltip_text.cpp
 *\brief Tooltip text helpers for palette and toolbar hover tips.
 */
#include "gui/hover_tooltip_text.h"

namespace Cgen
{
   namespace
   {
      constexpr float ApproxCharWidth = 7.0f;
   } // namespace

   void WrapHoverTooltipLines(std::string_view text,
                              float maxWidth,
                              std::vector<std::string>* pOutLines)
   {
      if (pOutLines == nullptr)
      {
         return;
      }
      pOutLines->clear();
      if (text.empty())
      {
         return;
      }

      const auto maxChars = static_cast<size_t>(maxWidth / ApproxCharWidth);
      const size_t limit = (maxChars < 12) ? 12 : maxChars;

      size_t index = 0;
      while (index < text.size())
      {
         while ((index < text.size()) &&
                ((text[index] == ' ') || (text[index] == '\n')))
         {
            ++index;
         }
         if (index >= text.size())
         {
            break;
         }

         size_t end = index;
         size_t lastBreak = index;
         while ((end < text.size()) && ((end - index) < limit))
         {
            if ((text[end] == ' ') || (text[end] == '\n'))
            {
               lastBreak = end;
            }
            if (text[end] == '\n')
            {
               break;
            }
            ++end;
         }

         if ((end < text.size()) && (lastBreak > index) &&
             ((end - index) >= limit))
         {
            end = lastBreak;
         }

         pOutLines->push_back(std::string(text.substr(index, end - index)));
         index = end;
      }
   }

   std::string_view PaletteRowHoverTipText(PaletteRowTipKind tipKind,
                                           BlockType blockType)
   {
      if (tipKind == PaletteRowTipKind::GroupHeader)
      {
         return "Click to expand or collapse this block group.";
      }
      return BlockTypeHelpText(blockType);
   }

   std::string_view ToolbarActionTooltipText(ToolbarAction action)
   {
      switch (action)
      {
         case ToolbarAction::NewDocument:
            return "New empty document.";
         case ToolbarAction::Open:
            return "Open a .cgen project.";
         case ToolbarAction::Save:
            return "Save the current project.";
         case ToolbarAction::Generate:
            return "Validate the graph and generate C99 into build_out/.";
         case ToolbarAction::Build:
            return "Compile the generated .c file with gcc.";
         case ToolbarAction::Run:
            return "Build and run; type stdin in Program Output.";
         case ToolbarAction::Stop:
            return "Stop the running program.";
         case ToolbarAction::Tidy:
            return "Auto-layout control flow left-to-right (Ctrl+L).";
         case ToolbarAction::Snap:
            return "Toggle grid snap and snap the current selection.";
         case ToolbarAction::AlignLeft:
            return "Align selected blocks to the leftmost X.";
         case ToolbarAction::AlignTop:
            return "Align selected blocks to the topmost Y.";
         case ToolbarAction::OrthogonalWires:
            return "Toggle orthogonal (elbow) wire routing for dense graphs.";
         case ToolbarAction::FitAll:
            return "Fit the whole graph in view (Ctrl+0).";
         case ToolbarAction::FitSelection:
            return "Fit the selection in view (Ctrl+Shift+0).";
         case ToolbarAction::Theme:
            return "Toggle Light / Dark theme.";
         case ToolbarAction::Help:
            return "Open in-app help (F1).";
         case ToolbarAction::None:
            return "";
      }
      return "";
   }
} // namespace Cgen
