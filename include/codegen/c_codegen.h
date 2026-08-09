/*!
 *\file c_codegen.h
 *\brief Generates C99 source from a flowchart document.
 */
#ifndef C_CODEGEN_H
#define C_CODEGEN_H

#include <string>

#include "model/graph_document.h"
#include "model/result.h"

namespace Cgen
{
   /*!
    *\brief Code generation outcome with optional diagnostics.
    */
   struct CodegenOutput
   {
      Result result = Result::Ok;
      std::string source;
      std::string diagnostics;
   };

   /*!
    *\brief Emits a complete C translation unit for the document.
    *
    *\param[in] document Flowchart document.
    *\return Generated source and status.
    */
   CodegenOutput GenerateCSource(const GraphDocument& document);

   /*!
    *\brief Emits a short C preview for one selected block.
    *
    *\param[in] document Flowchart document.
    *\param[in] nodeId Node to preview.
    *\return Preview text (may be empty).
    */
   std::string GenerateCSnippet(const GraphDocument& document, NodeId nodeId);
} // namespace Cgen

#endif // C_CODEGEN_H
