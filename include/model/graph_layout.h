/*!
 *\file graph_layout.h
 *\brief Simple left-to-right auto-layout for flowchart documents.
 */
#ifndef GRAPH_LAYOUT_H
#define GRAPH_LAYOUT_H

#include "model/graph_document.h"

namespace Cgen
{
   /*!
    *\brief Assigns layered left-to-right positions from Start and FunctionDefs.
    *
    *\param[in,out] pDocument Document to rearrange (must not be nullptr).
    */
   void ApplyAutoLayout(GraphDocument* pDocument);
} // namespace Cgen

#endif // GRAPH_LAYOUT_H
