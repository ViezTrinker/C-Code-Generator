/*!
 *\file document_history.h
 *\brief Undo/redo stacks of graph document snapshots.
 */
#ifndef DOCUMENT_HISTORY_H
#define DOCUMENT_HISTORY_H

#include <cstddef>
#include <vector>

#include "model/graph_document.h"

namespace Cgen
{
   /*!
    *\brief Stores checkpoints so graph edits can be undone and redone.
    */
   class DocumentHistory
   {
   public:
      /*!
       *\brief Clears all undo and redo checkpoints.
       */
      void Clear(void);

      /*!
       *\brief Pushes a checkpoint of the current graph and clears redo.
       *
       *\param[in] document Document to snapshot.
       */
      void PushCheckpoint(const GraphDocument& document);

      /*!
       *\brief Restores the most recent undo checkpoint.
       *
       *\param[in,out] pDocument Document to restore into.
       *\return true if a checkpoint was restored.
       */
      bool Undo(GraphDocument* pDocument);

      /*!
       *\brief Restores the most recent redo checkpoint.
       *
       *\param[in,out] pDocument Document to restore into.
       *\return true if a checkpoint was restored.
       */
      bool Redo(GraphDocument* pDocument);

      /*!
       *\brief Returns true when Undo can restore a checkpoint.
       */
      bool CanUndo(void) const;

      /*!
       *\brief Returns true when Redo can restore a checkpoint.
       */
      bool CanRedo(void) const;

   private:
      void PushLimited(std::vector<GraphSnapshot>* pStack, const GraphSnapshot& snapshot);

      static constexpr size_t MaxDepth = 64;
      std::vector<GraphSnapshot> _undoStack;
      std::vector<GraphSnapshot> _redoStack;
   };
} // namespace Cgen

#endif // DOCUMENT_HISTORY_H
