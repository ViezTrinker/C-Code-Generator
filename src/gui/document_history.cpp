/*!
 *\file document_history.cpp
 *\brief Undo/redo stack implementation.
 */
#include "gui/document_history.h"

namespace Cgen
{
   void DocumentHistory::Clear(void)
   {
      _undoStack.clear();
      _redoStack.clear();
   }

   void DocumentHistory::PushLimited(std::vector<GraphSnapshot>* pStack,
                                     const GraphSnapshot& snapshot)
   {
      if (pStack == nullptr)
      {
         return;
      }
      if (pStack->size() >= MaxDepth)
      {
         pStack->erase(pStack->begin());
      }
      pStack->push_back(snapshot);
   }

   void DocumentHistory::PushCheckpoint(const GraphDocument& document)
   {
      PushLimited(&_undoStack, document.CaptureGraph());
      _redoStack.clear();
   }

   bool DocumentHistory::Undo(GraphDocument* pDocument)
   {
      if ((pDocument == nullptr) || _undoStack.empty())
      {
         return false;
      }
      PushLimited(&_redoStack, pDocument->CaptureGraph());
      pDocument->RestoreGraph(_undoStack.back());
      _undoStack.pop_back();
      return true;
   }

   bool DocumentHistory::Redo(GraphDocument* pDocument)
   {
      if ((pDocument == nullptr) || _redoStack.empty())
      {
         return false;
      }
      PushLimited(&_undoStack, pDocument->CaptureGraph());
      pDocument->RestoreGraph(_redoStack.back());
      _redoStack.pop_back();
      return true;
   }

   bool DocumentHistory::CanUndo(void) const
   {
      return !_undoStack.empty();
   }

   bool DocumentHistory::CanRedo(void) const
   {
      return !_redoStack.empty();
   }
} // namespace Cgen
